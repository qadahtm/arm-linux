/*
 *  linux/arch/arm/mm/fault.c
 *
 *  Copyright (C) 1995  Linus Torvalds
 *  Modifications for ARM processor (c) 1995-2004 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/signal.h>
#include <linux/mm.h>
#include <linux/hardirq.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/uaccess.h>
#include <linux/page-flags.h>
#include <linux/sched.h>
#include <linux/highmem.h>
#include <linux/perf_event.h>

#include <asm/exception.h>
#include <asm/pgtable.h>
#include <asm/system_misc.h>
#include <asm/system_info.h>
#include <asm/tlbflush.h>

#include <asm/io.h>
#include <linux/slab.h>
#include <linux/ece695os.h>

#include "fault.h"

#ifdef CONFIG_MMU

#ifdef CONFIG_KPROBES
static inline int notify_page_fault(struct pt_regs *regs, unsigned int fsr)
{
	int ret = 0;

	if (!user_mode(regs)) {
		/* kprobe_running() needs smp_processor_id() */
		preempt_disable();
		if (kprobe_running() && kprobe_fault_handler(regs, fsr))
			ret = 1;
		preempt_enable();
	}

	return ret;
}
#else
static inline int notify_page_fault(struct pt_regs *regs, unsigned int fsr)
{
	return 0;
}
#endif

/*
 * This is useful to dump out the page tables associated with
 * 'addr' in mm 'mm'.
 */
void show_pte(struct mm_struct *mm, unsigned long addr)
{
	pgd_t *pgd;
        //printk(KERN_INFO "tqadah: shpw_pte(%08lx)",addr);
        
	if (!mm)
		mm = &init_mm;

	printk(KERN_ALERT "pgd = %p\n", mm->pgd);
	pgd = pgd_offset(mm, addr);
	printk(KERN_ALERT "[%08lx] *pgd=%08llx",
			addr, (long long)pgd_val(*pgd));

	do {
		pud_t *pud;
		pmd_t *pmd;
		pte_t *pte;

		if (pgd_none(*pgd))
			break;

		if (pgd_bad(*pgd)) {
			printk("(bad)");
			break;
		}

		pud = pud_offset(pgd, addr);
		if (PTRS_PER_PUD != 1)
			printk(", *pud=%08llx", (long long)pud_val(*pud));

		if (pud_none(*pud))
			break;

		if (pud_bad(*pud)) {
			printk("(bad)");
			break;
		}

		pmd = pmd_offset(pud, addr);
		if (PTRS_PER_PMD != 1)
			printk(", *pmd=%08llx", (long long)pmd_val(*pmd));

		if (pmd_none(*pmd))
			break;

		if (pmd_bad(*pmd)) {
			printk("(bad)");
			break;
		}

		/* We must not map this if we have highmem enabled */
		if (PageHighMem(pfn_to_page(pmd_val(*pmd) >> PAGE_SHIFT)))
			break;

		pte = pte_offset_map(pmd, addr);
		printk(", *pte=%08llx", (long long)pte_val(*pte));
#ifndef CONFIG_ARM_LPAE
		printk(", *ppte=%08llx",
		       (long long)pte_val(pte[PTE_HWTABLE_PTRS]));
#endif
		printk(", *hwpte=%08llx", (long long)readl(hw_pte(pte)));	/* xzl */
		pte_unmap(pte);
	} while(0);

	printk("\n");
}
#else					/* CONFIG_MMU */
void show_pte(struct mm_struct *mm, unsigned long addr)
{ }
#endif					/* CONFIG_MMU */


/* Returns the pte* given mm and addr.
 * This is modeled after show_pte().
 */
//static 
//pte_t *pte_lookup(struct mm_struct  *mm, int usermode, unsigned long addr)
pte_t *pte_lookup(struct task_struct  *task, int usermode, unsigned long addr)
{
	pgd_t *pgd; pmd_t *pmd; pte_t *pte;
        struct mm_struct * mm = task->active_mm;
//        if (addr == 0x40000000)
//            printk("looking up addr = %08lx \n",addr);
	pgd = pgd_offset(mm, addr);

//	printk("gets pgd @ %08x\n", (u32)pgd);

	if (!pgd || !pgd_present(*pgd)) {
//		printk("bad pgd\n");
		return NULL;
	}

	pmd = pmd_offset(pud_offset(pgd, addr), addr);
	if (!pmd || !pmd_present(*pmd)) {
//		printk("bad pmd\n");	// this can happen due to on-demand paging
		return NULL;
	}
        
//        if (addr == 0x40000000)
//	printk("pmd val %08x\n", *pmd);

	pte = pte_offset_map(pmd, addr);
	if (!pte) {
//		printk("fail to get pte\n");
		return NULL;
	}

	if (!pte_present(*pte)) {
            if (addr == 0x40000000)
                    printk( "pte_lookup: bad pte val %08x, addr(%08lx), task pid(%d)\n", *pte, addr,task->pid);  // can happen
		return NULL;
	}
        
        if ((usermode && !pte_present_user(*pte))){
            if (addr == 0x40000000)
                printk( "pte_lookup user == 1: bad pte val %08x, addr(%08lx), task pid(%d)\n", *pte, addr,task->pid);  // can happen
		
            return NULL;
        }

	return pte;
}


struct refcount * refc_lookup(struct task_struct *task, pte_t * pte){
    struct refcount* iter = NULL;
    
    iter = task->refcount_head;
    while (iter != NULL) {
        if (*(iter->pte) == *pte) {
            printk("refc_lookup: found refc for pte(%08lx), refcount = %d\n", (unsigned long) *pte, iter->n);
            return iter;
        }
        iter = iter->next;
    }
    return NULL;
}

/* Mark hardware pte as invalid so that future memory access will trigger
 * exception.
 */
//void mask_hwpte(pte_t *pte)
void mask_hwpte(struct refcount * refc)
{
	unsigned long hpte;
        unsigned long chpte;
        unsigned long ohpte;
        
	hpte = readl(hw_pte(refc->pte));
        refc->hpte = hpte;
        chpte = (hpte & ~0x2);
        ohpte = (hpte & ~0x3);
        
//	printk( "mask_hwpte: pte(%08lx), hpte was %08lx, going to disable it.\n", (unsigned long)*pte,(unsigned long)hpte);
        // 
        
        if (refc->page_type == 1){
            printk(  "mask_hwpte: (code page) addr(%08lx)  pte(%08lx), hpte was %08lx, going to disable it with masked_hpte(%08lx).\n"
                    ,refc->vaddr, (unsigned long)*(refc->pte),(unsigned long)hpte, chpte);
            writel(chpte, (void *)hw_pte(refc->pte));
        }
        else if (refc->page_type == 0){
            printk(  "mask_hwpte: (data page) addr(%08lx)  pte(%08lx), hpte was %08lx, going to disable it with masked_hpte(%08lx).\n"
                    ,refc->vaddr, (unsigned long)*(refc->pte),(unsigned long)hpte, ohpte);
            writel(ohpte, (void *)hw_pte(refc->pte));
        }
        else{
            printk(  "mask_hwpte: (other page) addr(%08lx) pte(%08lx), hpte was %08lx, going to disable it with masked_hpte(%08lx).\n"
                    ,refc->vaddr, (unsigned long)*(refc->pte),(unsigned long)hpte,ohpte);
            writel(ohpte, (void *)hw_pte(refc->pte));
        }
        
//        writel((hpte & ~0x3) , (void *)hw_pte(refc->pte));
        
	/* We've made changes to hwpte. Flush the changes */
	clean_dcache_area((void *)(hw_pte(refc->pte)), sizeof(pte_t));   /* right API? */

	/* flush tlb. Necessary. Othwerise future accesses may use stale TLB
	 * for xslat which cannot be trapped. */
	flush_tlb_kernel_page(__phys_to_virt(hpte & PAGE_MASK));
        refc->valid_page = 0;
        printk( "mask_hwpte: pte(%08lx), hpte was (%08lx), chpte is (%08lx), ohpte is (%08lx), addr(%08lx) is disabled.\n"
                , (unsigned long)*(refc->pte),hpte, chpte,ohpte, refc->vaddr);
}

/* Mark hardware pte as valid so that future memory access will NOT trigger
 * exception.
 */
//int unmask_hwpte(pte_t *pte)
int unmask_hwpte(struct refcount * refc)
{
	unsigned long hpte;

	hpte = readl(hw_pte(refc->pte));
//	printk( "unmask_hwpte: pte(%08lx), hpte was %08lx. going to enable it.\n",(unsigned long)*pte, (unsigned long)hpte);

        writel(refc->hpte, (void *)hw_pte(refc->pte));
//	writel(hpte | 0x3, (void *)hw_pte(pte));
	clean_dcache_area((void *)(hw_pte(refc->pte)), sizeof(pte_t)); /*bring TLB in sync. no $inv*/
//	flush_tlb_kernel_page(__phys_to_virt(hpte & PAGE_MASK)); /* necessary? */
        if (refc->page_type == 1){
            printk(  "unmask_hwpte (code page): pte(%08lx), hpte was (%08lx), restored hpte(%08lx), addr(%08lx). is enabled.\n"
                ,(unsigned long)*(refc->pte), (unsigned long)hpte, refc->hpte,refc->vaddr);
        }
        else if (refc->page_type == 0){
            printk(  "unmask_hwpte (data page): pte(%08lx), hpte was (%08lx), restored hpte(%08lx), addr(%08lx). is enabled.\n"
                ,(unsigned long)*(refc->pte), (unsigned long)hpte, refc->hpte,refc->vaddr);
        }
        else{
            
            printk(  "unmask_hwpte (other page): pte(%08lx), hpte was (%08lx), restored hpte(%08lx), addr(%08lx). is enabled.\n"
                ,(unsigned long)*(refc->pte), (unsigned long)hpte, refc->hpte,refc->vaddr);
        }
        refc->valid_page = 1;
	return 1;
}

void ece695_mask_page_abs(unsigned long vaddr) {}

void ece695_mask_page(struct task_struct *tsk, unsigned long vaddr)
{
    pte_t * pte;
    
//    if (tsk->active_mm->start_code <= vaddr && tsk->active_mm->end_code >= vaddr){
//        printk( "mask_page: trying to mask %08lx which belong to code page, skip masking code pages\n", vaddr);
//        return;
//    }
    
//    printk( "mask_page: trying to mask  %08lx\n", vaddr);
    //    pte = pte_lookup(tsk->mm, 1, vaddr);
    pte = pte_lookup(tsk, 1, vaddr);
//    printk( "mask_page: pte lookup returns %08lx for vaddr(%08lx) with task pid(%d)\n", 
//            (unsigned long)*pte,vaddr,tsk->pid);
    if (pte == NULL) {
            printk( "[ERROR] pte_lookup() failed\n");
            return;
    }
    BUG();
//    mask_hwpte(pte);                 

}

/* Replace the next user instruction as a special Undefined instruction, which
 * will trigger exception and thus trap into the kernel again.
 *
 * This allows us to execute the faulty instruction, while letting us to turn
 * the pte back into invalid right after the faulty instruction is executed.
 */
void patch_instr(struct refcount* refc, unsigned long addr,struct task_struct * task , struct pt_regs *regs)
{
	/* see do_undefinstr() for a systematic way of doing this */
    
	unsigned long pc  = regs->ARM_pc;
	unsigned long instr;	/* the faulty instr */
	unsigned long undef = BUG_INSTR_VALUE;
	pte_t * pte;
	unsigned long hpte;
        struct mm_struct *mm = task->mm;
        
        /* Read the faulty instruction from pc */
	if (get_user(instr, (u32 __user *)pc) == 0){
		printk( "patch_instr: original faulty pc %08lx, instr %08lx addr(%08lx)\n", (unsigned long)pc,(unsigned long) instr, addr);
        }
        else{
            printk( "patch_instr: could not get user instruction pc %08lx, instr %08lx addr(%08lx)\n", (unsigned long)pc,(unsigned long) instr, addr);
        }
        
        if (instr == undef){
            
            printk( "patch_instr: trying to patch the patch i.e. the (undef) instruction that we planted, return without patching addr(%08lx)",addr);
            return;
        }
        
	/*
	 * Make the instruction page r/w so that we can plant the undefinstr.
	 * Originally, the instruction page should be r/o.
	 */
	printk( "patch_instr: patching for addr (%08lx), pc(%08lx)\n",(unsigned long)addr,(unsigned long)pc);
        pte = pte_lookup(task, 1, pc);
//        printk( "patch_instr: pte lookup returned (%08lx) addr (%08lx), pc(%08lx)\n",
//                (unsigned long) *pte,(unsigned long)addr,(unsigned long)pc);
	if (!pte) {
		printk( "patch_instr: bug -- why no pte for the faulty pc?\n");        
		BUG();
	}
        printk( "patch_instr: found pte(%08lx) for pc(%08lx)\n",(unsigned long)*pte,(unsigned long) pc);
        

#if 0	/* debugging */
	if (pte_write(*pte))
		printk("page r/w\n");
	else
		printk("page r/o\n");
#endif

	hpte = readl(hw_pte(pte));

#if 1	/* debugging */
	printk( "patch_instr: hpte %08lx PTE_EXT_APX %ld PTE_EXT_AP1 %ld\n",(unsigned long) hpte,
			 PTE_EXT_APX & hpte, PTE_EXT_AP1 & hpte);
#endif

	set_pte_at(mm, addr, pte, pte_mkwrite(*pte));
	writel(hpte & (~PTE_EXT_APX), (void *)hw_pte(pte));

//	flush_cache_all();	/* cannot flush, why? */

	printk( " --- after change --- \n");

#if 0	/* debugging */
	if (pte_write(*pte))
		printk("page r/w\n");
	else
		printk("page r/o\n");
#endif

	hpte = readl(hw_pte(pte));
	printk( "patch_instr:  hpte %08lx PTE_EXT_APX %ld\n", (unsigned long) hpte,  PTE_EXT_APX & hpte);

#if 0	/* debugging */
	if (!access_ok(VERIFY_WRITE, (u32 __user *)pc, 4))
		printk("access_ok failed\n");
	else
		printk("access_ok okay\n");
#endif

	/* XXX determine the next instruction.
	 *
	 * in theory, we have to decode the faulty instr and see what is
	 * the next instr. here, we simply assume the next instr is at
	 * pc + 4 (which may not be always true, e.g. ldr pc, [r0])
	 */
        
        // check for thumb
        if (thumb_mode(regs)){
            printk( "patch_instr:  thumb instruction at pc (%08lx) instr(%08lx) addr(%08lx)\n"
                    ,(unsigned long)pc, instr, addr);
            
            if (!is_wide_instruction(instr)) {
                printk( "patch_instr: half-word thumb instruction at pc (%08lx) instr(%08lx)\n",(unsigned long)pc, instr);
                pc += 2;
                BUG();
                goto endpatch;
            }
            else {
                printk( "patch_instr:  wide thumb instruction at pc (%08lx) instr(%08lx) addr(%08lx) will not patch\n"
                        ,(unsigned long)pc, instr, addr);
//                printk( "patch_instr:  wide thumb instruction at pc (%08lx) instr(%08lx) \n",(unsigned long)pc, instr);
                
                goto endpatch;
            }

        }
        else {
            printk( "patch_instr: arm instruction at pc (%08lx) instr(%08lx) addr(%08lx)\n", (unsigned long)pc,instr, addr);
        }
        
        pc += 4;
        
        refc->saved_pc = pc; 	/* undefinstr handler will need this */
        /* save the next user instruction */
        if (get_user(refc->saved_instr, (u32 __user *)pc) == 0)
                printk( "patch_instr: to patch @%08lx, saved instr %08lx , addr(%08lx)\n", (unsigned long) pc, (unsigned long)refc->saved_instr, addr);
        else {
                printk( "patch_instr: bug -- cannot read instr at %08lx\n", (unsigned long) pc);
                BUG();
        }

        /* overwrite the next user instruction as an undef instr */
        if (put_user(undef, (u32 __user *)pc) == 0)
                printk( "patch_instr: write undef instr ok\n");
        else {
                printk( "patch_instr: bug -- cannot write the undef instr. \n");
                BUG();
        }
        
        

#if 0
	if (__copy_to_user((u32 __user *)pc, &undef, sizeof(unsigned int)) == 0)
		printk("__copy_to_user okay\n");
	else
		printk("__copy_to_user failed\n");
#endif

                
        
        endpatch:
//	__flush_icache_all();
	flush_cache_all();	// <- xzl: really needed? XXX
        
	get_user(instr, (u32 __user *)pc);
	printk( "patch_instr: read again: faulty pc %08lx, instr %08lx , undef (%08lx) addr(%08lx)\n"
                , (unsigned long) pc,(unsigned long) instr, undef, addr);
	printk( "patch_instr: patch is done for addr(%08lx)\n",addr);
        
}

/* will be invoked upon undefinstr exception.
 * return 0 on success */
int ece695_restore_saved_instr(struct pt_regs *regs)
{
	unsigned int pc;
	unsigned int instr;
        struct refcount * refc;
        
        
//        printk( "restore  instr :task pid(%d).\n",(int)current->pid);
        
//	pc = (void __user *)instruction_pointer(regs);
        pc = (unsigned int)((void __user *)instruction_pointer(regs));

//        printk( "restore  instr : lookup refc for pc = %08lx\n",(unsigned long)pc);
        
        refc = current->refcount_head;        
        if (current->refcount_head == NULL) {
            printk( "restore  instr : refcount entry not initialized, not us. pass it up.\n");
            return -1;
        }
        
        while (refc != NULL){
            if (refc->saved_pc == pc){
                printk( "restore  instr : found matching pc = %08lx, vaddr = %08lx, pte = %08lx, refcount = %d\n"
                        ,(unsigned long) pc,refc->vaddr, (unsigned long) *(refc->pte),refc->n);
                break;
            }
            refc = refc->next;
        }
        
        if (refc == NULL){
            printk( "restore  instr : cannot find corresponding refc entry for pc(%08x)\n",pc);
            return -1;
        }
        else{
            printk( "restore  instr : found corresponding refc entry for pc(%08x) , addr(%08lx)\n",pc,refc->vaddr);
        }
        
        if ((!refc->saved_instr) || (!refc->saved_pc)) {
		printk( "restore  instr : no saved instr found -- why?\n");
		return -1;
	}
                
	printk( "restore  instr : oh this undefinstr is planted by us. \n");
	get_user(instr, (u32 __user *)pc);
	printk( "restore  instr : read: faulty pc %08x, instr %08x\n", pc, instr);

	if (put_user(refc->saved_instr, (u32 __user *)pc) == 0)
		printk( "restore instr ok -- user good to go\n");
	else {
		printk( "bug -- cannot restore instr. \n");
		BUG();
	}

	/* XXX: make the instruction page as r/o for security */

	get_user(instr, (u32 __user *)pc);
	printk( "restore  instr : read again: faulty pc %08x, instr %08x\n", pc, instr);

	refc->saved_instr = 0;
	refc->saved_pc = 0;

	if (!(refc->pte)) {
		printk( "bug -- no saved pte?\n");
		BUG();
	}
        
        // mask to capture next access
        mask_hwpte(refc);
	return 0;
}

struct refcount* update_refcount(struct task_struct *tsk, pte_t * pte, unsigned long addr, struct pt_regs *regs) {
    struct refcount* newref = NULL;
    struct refcount* refc = NULL;
    unsigned int cref =0;
    
    if (tsk->refcount_head == NULL) {
        printk( "update_refcount: no refcount data for the whole task. This should not be the case WARNNING\n");
        if (NULL == (newref =
                (struct refcount *) kzalloc(
                sizeof (struct refcount), GFP_KERNEL))) {
            printk( "[ERROR] kzalloc() failed. 1\n");
            BUG();
        }
        newref->n = 1;
        newref->vaddr = addr;
        newref->pte = pte;
        newref->saved_instr = 0;
        newref->saved_pc = 0;
        newref->next = NULL;
        tsk->refcount_head = newref;
        tsk->refcount_tail = newref;
        cref = newref->n;
        refc = newref;
    } else {
        refc = refc_lookup(tsk,pte);
        
        if (refc == NULL) {
            // refc not found for pte (unlikely)
            printk( "update_refcount: no refcount data for pte(%08lx), addr(%08lx). This should not be the case WARNNING\n"
                    ,(unsigned long)*pte,addr);
            if (NULL == (newref =
                    (struct refcount *) kzalloc(
                    sizeof (struct refcount), GFP_KERNEL))) {
                printk( "[ERROR] kzalloc() failed. 2\n");
                BUG();
            }
            newref->n = 1;
            newref->vaddr = addr;
            newref->pte = pte;
            newref->saved_instr = 0;
            newref->saved_pc = 0;
            newref->next = NULL;
            tsk->refcount_tail->next = newref;
            tsk->refcount_tail = newref;
            cref = newref->n;
            refc = newref;
        }
        else{
            // found
            refc->n += 1;
            cref = refc->n;
            
            printk(  "update_refcount: incremented count for addr(%08lx) pte(%08lx), refcount = %d\n", addr, (unsigned long) *pte, cref);                
        }
    }
    
    if (refc->page_type == 1){
        // umask code page before patching
        printk( "===> (code page) Woo-hoo! caught an access. refcount=%d for addr = %08lx, pte(%08lx)\n"
                , cref, addr, (unsigned long) *pte);
//        unmask_hwpte(refc);
    }
    else if (refc->page_type == 0) {
        printk( "===> (data_page) Woo-hoo! caught an access. refcount=%d for addr = %08lx, pte(%08lx)\n"
                , cref, addr, (unsigned long) *pte);
    }
    else{
       printk( "===> (other_page) Woo-hoo! caught an access. refcount=%d for addr = %08lx, pte(%08lx)\n"
                , cref, addr, (unsigned long) *pte); 
    }
    
    if (cref < 10) {
        printk( "update_refcount: going to patch instruction for addr(%08lx)\n", addr);
        patch_instr(refc, addr, tsk, regs);
        
    } else
        printk( "update_refcount: refcount large enough. stop monitoring\n");
           
    
    
    return refc;
}

/*
 * Oops.  The kernel tried to access some page that wasn't present.
 */
static void
__do_kernel_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr,
		  struct pt_regs *regs)
{
	/*
	 * Are we prepared to handle this kernel fault?
	 */
	if (fixup_exception(regs))
		return;

	/*
	 * No handler, we'll have to terminate things with extreme prejudice.
	 */
	bust_spinlocks(1);
	printk(KERN_ALERT
		"Unable to handle kernel %s at virtual address %08lx\n",
		(addr < PAGE_SIZE) ? "NULL pointer dereference" :
		"paging request", addr);

	show_pte(mm, addr);
	die("Oops", regs, fsr);
	bust_spinlocks(0);
	do_exit(SIGKILL);
}

/*
 * Something tried to access memory that isn't in our memory map..
 * User mode accesses just cause a SIGSEGV
 */
static void
__do_user_fault(struct task_struct *tsk, unsigned long addr,
		unsigned int fsr, unsigned int sig, int code,
		struct pt_regs *regs)
{
	struct siginfo si;

#ifdef CONFIG_DEBUG_USER
	if (((user_debug & UDBG_SEGV) && (sig == SIGSEGV)) ||
	    ((user_debug & UDBG_BUS)  && (sig == SIGBUS))) {
		printk(KERN_DEBUG "%s: unhandled page fault (%d) at 0x%08lx, code 0x%03x\n",
		       tsk->comm, sig, addr, fsr);
		show_pte(tsk->mm, addr);
		show_regs(regs);
	}
#endif

	tsk->thread.address = addr;
	tsk->thread.error_code = fsr;
	tsk->thread.trap_no = 14;
	si.si_signo = sig;
	si.si_errno = 0;
	si.si_code = code;
	si.si_addr = (void __user *)addr;
	force_sig_info(sig, &si, tsk);
}

void do_bad_area(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	struct task_struct *tsk = current;
	struct mm_struct *mm = tsk->active_mm;

	/*
	 * If we are in kernel mode at this point, we
	 * have no context to handle this fault with.
	 */
	if (user_mode(regs))
		__do_user_fault(tsk, addr, fsr, SIGSEGV, SEGV_MAPERR, regs);
	else
		__do_kernel_fault(mm, addr, fsr, regs);
}

#ifdef CONFIG_MMU
#define VM_FAULT_BADMAP		0x010000
#define VM_FAULT_BADACCESS	0x020000

/*
 * Check that the permissions on the VMA allow for the fault which occurred.
 * If we encountered a write fault, we must have write permission, otherwise
 * we allow any permission.
 */
static inline bool access_error(unsigned int fsr, struct vm_area_struct *vma)
{
	unsigned int mask = VM_READ | VM_WRITE | VM_EXEC;

	if (fsr & FSR_WRITE)
		mask = VM_WRITE;
	if (fsr & FSR_LNX_PF)
		mask = VM_EXEC;

	return vma->vm_flags & mask ? false : true;
}

static int __kprobes
__do_page_fault(struct mm_struct *mm, unsigned long addr, unsigned int fsr,
		unsigned int flags, struct task_struct *tsk)
{
	struct vm_area_struct *vma;
	int fault;

	vma = find_vma(mm, addr);
	fault = VM_FAULT_BADMAP;
	if (unlikely(!vma))
		goto out;
	if (unlikely(vma->vm_start > addr))
		goto check_stack;

	/*
	 * Ok, we have a good vm_area for this
	 * memory access, so we can handle it.
	 */
good_area:
	if (access_error(fsr, vma)) {
		fault = VM_FAULT_BADACCESS;
		goto out;
	}

	return handle_mm_fault(mm, vma, addr & PAGE_MASK, flags);

check_stack:
	/* Don't allow expansion below FIRST_USER_ADDRESS */
	if (vma->vm_flags & VM_GROWSDOWN &&
	    addr >= FIRST_USER_ADDRESS && !expand_stack(vma, addr))
		goto good_area;
out:
	return fault;
}

static int __kprobes
do_page_fault(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	struct task_struct *tsk;
	struct mm_struct *mm;
	int fault, sig, code;
        struct refcount *refc;
        pte_t *pte;
        unsigned long hpte;
	unsigned int flags = FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;
        
	
        if (notify_page_fault(regs, fsr))
		return 0;

	tsk = current;
	mm  = tsk->mm;

        //        printk( "tqadah:do page fault addr(%08lx)\n",addr);
        
#if 0
      if (strncmp(tsk->comm, "xzltestprog", TASK_COMM_LEN) == 0) {
//                    printk( "page fault handling for xzltestprog addr(%08lx)\n",addr);
			pte = pte_lookup(tsk, 1, addr);
//                        printk( "page fault: pte val returned is %08lx, addr(%08lx) task pid(%d)\n", 
//                                (unsigned long) pte, addr, tsk->pid);
			if (pte && (refc= refc_lookup(tsk,pte))) {
                            printk( "page fault: we have a refcount for (%08lx), pte(%08lx)\n",addr, (unsigned long) *pte);
                            //printk( "page fault: reading hwpte addr(%08lx)\n",addr);
				hpte = readl(hw_pte(pte));
//                                printk( "page fault: read hwpte(%08lx) for addr(%08lx)\n", hpte,addr);
//				if (hpte && (!(hpte & 0x3))) {
					/* hw pte has bit set, but invalid. seems planted by us */
//					printk( "===> (code page) Woo-hoo! caught an access. refcount=%d\n", ++ref);
//                                        printk( "page fault: passed test for hwpte(%08lx) for addr(%08lx), our doing\n", hpte,addr);
                                        
                                        if (refc->page_type == 1){
                                            // code page
                                            printk( "page fault: got a code page at (%08lx)\n",addr);
                                            unmask_hwpte(refc);
                                            refc = update_refcount(tsk,pte,addr,regs);
                                            // no need to unmask because it was already done inside update_refcoount
                                            return 0;
                                        }
                                        else 
                                            //if (addr >= tsk->active_mm->start_data && addr <= tsk->active_mm->end_data)
                                        {
                                        
                                            // data page
                                            printk( "page fault: got a data page at (%08lx)\n",addr);
                                            
                                            refc = update_refcount(tsk,pte,addr,regs);
                                            unmask_hwpte(refc);
                                            return 0;
//                                        }
//                                        else{
//                                        // no patching for other pages, i.e. disable counting
//                                            printk( "page fault: no patching for (%08lx)\n",addr);
//                                            unmask_hwpte(refc);
//                                            return 0;
                                        }
					
					/* let user to execute the faulty instr */
//                                        refc = refc_lookup(tsk,pte);
//					unmask_hwpte(refc);
//					return 0;	/* skip Linux's handling */
//				}
			}
//                        else{
//                            printk( "page fault: pte_lookup failed for addr(%08lx)\n",addr);
//                        }
		}  
        
#endif
        
	/* Enable interrupts if they were enabled in the parent context. */
	if (interrupts_enabled(regs))
		local_irq_enable();

	/*
	 * If we're in an interrupt or have no user
	 * context, we must not take the fault..
	 */
	if (in_atomic() || !mm)
		goto no_context;

	if (user_mode(regs))
		flags |= FAULT_FLAG_USER;
	if (fsr & FSR_WRITE)
		flags |= FAULT_FLAG_WRITE;

	/*
	 * As per x86, we may deadlock here.  However, since the kernel only
	 * validly references user space from well defined areas of the code,
	 * we can bug out early if this is from code which shouldn't.
	 */
	if (!down_read_trylock(&mm->mmap_sem)) {
		if (!user_mode(regs) && !search_exception_tables(regs->ARM_pc))
			goto no_context;
retry:
		down_read(&mm->mmap_sem);
	} else {
		/*
		 * The above down_read_trylock() might have succeeded in
		 * which case, we'll have missed the might_sleep() from
		 * down_read()
		 */
		might_sleep();
#ifdef CONFIG_DEBUG_VM
		if (!user_mode(regs) &&
		    !search_exception_tables(regs->ARM_pc))
			goto no_context;
#endif
	}

	fault = __do_page_fault(mm, addr, fsr, flags, tsk);

	/* If we need to retry but a fatal signal is pending, handle the
	 * signal first. We do not need to release the mmap_sem because
	 * it would already be released in __lock_page_or_retry in
	 * mm/filemap.c. */
	if ((fault & VM_FAULT_RETRY) && fatal_signal_pending(current))
		return 0;

	/*
	 * Major/minor page fault accounting is only done on the
	 * initial attempt. If we go through a retry, it is extremely
	 * likely that the page will be found in page cache at that point.
	 */

	perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS, 1, regs, addr);
	if (!(fault & VM_FAULT_ERROR) && flags & FAULT_FLAG_ALLOW_RETRY) {
		if (fault & VM_FAULT_MAJOR) {
			tsk->maj_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MAJ, 1,
					regs, addr);
		} else {
			tsk->min_flt++;
			perf_sw_event(PERF_COUNT_SW_PAGE_FAULTS_MIN, 1,
					regs, addr);
		}
		if (fault & VM_FAULT_RETRY) {
			/* Clear FAULT_FLAG_ALLOW_RETRY to avoid any risk
			* of starvation. */
			flags &= ~FAULT_FLAG_ALLOW_RETRY;
			flags |= FAULT_FLAG_TRIED;
			goto retry;
		}
	}

	up_read(&mm->mmap_sem);

	/*
	 * Handle the "normal" case first - VM_FAULT_MAJOR / VM_FAULT_MINOR
	 */
	if (likely(!(fault & (VM_FAULT_ERROR | VM_FAULT_BADMAP | VM_FAULT_BADACCESS))))
		return 0;

	/*
	 * If we are in kernel mode at this point, we
	 * have no context to handle this fault with.
	 */
	if (!user_mode(regs))
		goto no_context;

	if (fault & VM_FAULT_OOM) {
		/*
		 * We ran out of memory, call the OOM killer, and return to
		 * userspace (which will retry the fault, or kill us if we
		 * got oom-killed)
		 */
		pagefault_out_of_memory();
		return 0;
	}

	if (fault & VM_FAULT_SIGBUS) {
		/*
		 * We had some memory, but were unable to
		 * successfully fix up this page fault.
		 */
		sig = SIGBUS;
		code = BUS_ADRERR;
	} else {
		/*
		 * Something tried to access memory that
		 * isn't in our memory map..
		 */
		sig = SIGSEGV;
		code = fault == VM_FAULT_BADACCESS ?
			SEGV_ACCERR : SEGV_MAPERR;
	}

	__do_user_fault(tsk, addr, fsr, sig, code, regs);
	return 0;

no_context:
	__do_kernel_fault(mm, addr, fsr, regs);
	return 0;
}
#else					/* CONFIG_MMU */
static int
do_page_fault(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	return 0;
}
#endif					/* CONFIG_MMU */

/*
 * First Level Translation Fault Handler
 *
 * We enter here because the first level page table doesn't contain
 * a valid entry for the address.
 *
 * If the address is in kernel space (>= TASK_SIZE), then we are
 * probably faulting in the vmalloc() area.
 *
 * If the init_task's first level page tables contains the relevant
 * entry, we copy the it to this task.  If not, we send the process
 * a signal, fixup the exception, or oops the kernel.
 *
 * NOTE! We MUST NOT take any locks for this case. We may be in an
 * interrupt or a critical region, and should only copy the information
 * from the master page table, nothing more.
 */
#ifdef CONFIG_MMU
static int __kprobes
do_translation_fault(unsigned long addr, unsigned int fsr,
		     struct pt_regs *regs)
{
	unsigned int index;
	pgd_t *pgd, *pgd_k;
	pud_t *pud, *pud_k;
	pmd_t *pmd, *pmd_k;

	if (addr < TASK_SIZE)
		return do_page_fault(addr, fsr, regs);

	if (user_mode(regs))
		goto bad_area;

	index = pgd_index(addr);

	pgd = cpu_get_pgd() + index;
	pgd_k = init_mm.pgd + index;

	if (pgd_none(*pgd_k))
		goto bad_area;
	if (!pgd_present(*pgd))
		set_pgd(pgd, *pgd_k);

	pud = pud_offset(pgd, addr);
	pud_k = pud_offset(pgd_k, addr);

	if (pud_none(*pud_k))
		goto bad_area;
	if (!pud_present(*pud)) {
		set_pud(pud, *pud_k);
		/*
		 * There is a small window during free_pgtables() where the
		 * user *pud entry is 0 but the TLB has not been invalidated
		 * and we get a level 2 (pmd) translation fault caused by the
		 * intermediate TLB caching of the old level 1 (pud) entry.
		 */
		flush_tlb_kernel_page(addr);
	}

	pmd = pmd_offset(pud, addr);
	pmd_k = pmd_offset(pud_k, addr);

#ifdef CONFIG_ARM_LPAE
	/*
	 * Only one hardware entry per PMD with LPAE.
	 */
	index = 0;
#else
	/*
	 * On ARM one Linux PGD entry contains two hardware entries (see page
	 * tables layout in pgtable.h). We normally guarantee that we always
	 * fill both L1 entries. But create_mapping() doesn't follow the rule.
	 * It can create inidividual L1 entries, so here we have to call
	 * pmd_none() check for the entry really corresponded to address, not
	 * for the first of pair.
	 */
	index = (addr >> SECTION_SHIFT) & 1;
#endif
	if (pmd_none(pmd_k[index]))
		goto bad_area;
	if (!pmd_present(pmd[index]))
		copy_pmd(pmd, pmd_k);

	return 0;

bad_area:
	do_bad_area(addr, fsr, regs);
	return 0;
}
#else					/* CONFIG_MMU */
static int
do_translation_fault(unsigned long addr, unsigned int fsr,
		     struct pt_regs *regs)
{
	return 0;
}
#endif					/* CONFIG_MMU */

/*
 * Some section permission faults need to be handled gracefully.
 * They can happen due to a __{get,put}_user during an oops.
 */
static int
do_sect_fault(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
    printk( "tqadah:do sect fault addr(%08lx)\n",addr);
	do_bad_area(addr, fsr, regs);
	return 0;
}

/*
 * This abort handler always returns "fault".
 */
static int
do_bad(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
    printk( "tqadah:do bad addr(%08lx)\n",addr);
	return 1;
}

struct fsr_info {
	int	(*fn)(unsigned long addr, unsigned int fsr, struct pt_regs *regs);
	int	sig;
	int	code;
	const char *name;
};

/* FSR definition */
#ifdef CONFIG_ARM_LPAE
#include "fsr-3level.c"
#else
#include "fsr-2level.c"
#endif

void __init
hook_fault_code(int nr, int (*fn)(unsigned long, unsigned int, struct pt_regs *),
		int sig, int code, const char *name)
{
	if (nr < 0 || nr >= ARRAY_SIZE(fsr_info))
		BUG();

	fsr_info[nr].fn   = fn;
	fsr_info[nr].sig  = sig;
	fsr_info[nr].code = code;
	fsr_info[nr].name = name;
}

/*
 * Dispatch a data abort to the relevant handler.
 */
asmlinkage void __exception
do_DataAbort(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
	const struct fsr_info *inf = fsr_info + fsr_fs(fsr);
	struct siginfo info;

	/* xzl: add code here */
	struct task_struct *tsk;
	struct mm_struct *mm;
	pte_t *pte;
        struct refcount * refc;
	unsigned long hpte;

	

	/* xzl: we have a fault; is the fault caused by us? */
	// (fsr & FSR_FS3_0) == PAGE_TRANSLATION_FAULT) ??
#if 1	/* simple heuristics. you can go fancy of course */
        
	if (current) {
		tsk = current; 
		if (strncmp(tsk->comm, "xzltestprog", TASK_COMM_LEN) == 0) {
//                    printk( "data abort handling for xzltestprog addr(%08lx)\n",addr);
			pte = pte_lookup(tsk, 1, addr);
//                        printk( "data abort: pte val returned is %08lx, addr(%08lx) task pid(%d)\n", 
//                                (unsigned long) pte, addr, tsk->pid);
                        //refc= refc_lookup(tsk,pte);
			if (pte) {                            
//                            printk( "data abort: reading hwpte addr(%08lx)\n",addr);
				hpte = readl(hw_pte(pte));
//                                printk( "data abort: read hwpte(%08lx) for addr(%08lx)\n", hpte,addr);
				if (hpte && (!(hpte & 0x3))) {
					/* hw pte has bit set, but invalid. seems planted by us */
                                        printk( "data abort: passed test for hwpte(%08lx) for addr(%08lx), our doing\n", hpte,addr); 
					refc = update_refcount(tsk,pte,addr,regs);
					/* let user to execute the faulty instr */
					unmask_hwpte(refc); // done in update_refcount
                                        
					return;	/* skip Linux's handling */
				}
                                else{
                                   printk( "data abort: did not pass test for hwpte(%08lx) for addr(%08lx), our doing\n", hpte,addr); 
                                }
			}
                        else{
                            printk( "data abort: pte_lookup failed for addr(%08lx)\n",addr);
                        }
		}
	}
#endif

	if (!inf->fn(addr, fsr & ~FSR_LNX_PF, regs)) {
		return;
	}

	printk(KERN_ALERT "Unhandled fault: %s (0x%03x) at 0x%08lx\n",
		inf->name, fsr, addr);

	info.si_signo = inf->sig;
	info.si_errno = 0;
	info.si_code  = inf->code;
	info.si_addr  = (void __user *)addr;
	arm_notify_die("", regs, &info, fsr, 0);
}

void __init
hook_ifault_code(int nr, int (*fn)(unsigned long, unsigned int, struct pt_regs *),
		 int sig, int code, const char *name)
{
	if (nr < 0 || nr >= ARRAY_SIZE(ifsr_info))
		BUG();

	ifsr_info[nr].fn   = fn;
	ifsr_info[nr].sig  = sig;
	ifsr_info[nr].code = code;
	ifsr_info[nr].name = name;
}

asmlinkage void __exception
do_PrefetchAbort(unsigned long addr, unsigned int ifsr, struct pt_regs *regs)
{
	const struct fsr_info *inf = ifsr_info + fsr_fs(ifsr);
	struct siginfo info;
        struct refcount * refc;
        unsigned long hpte;
        pte_t *pte;
        struct task_struct * task;
        
#if 1	
        
	if (current) {
		task = current; 
		if (strncmp(task->comm, "xzltestprog", TASK_COMM_LEN) == 0) {
//                    printk( "prefetch abort handling for xzltestprog addr(%08lx)\n",addr);
			pte = pte_lookup(task, 1, addr);
//                        printk( "prefetch abort: pte val returned is %08lx, addr(%08lx) task pid(%d)\n", 
//                                (unsigned long) pte, addr, task->pid);
                        
			if (pte) {                            
                            refc= refc_lookup(task,pte);
                            printk(  "prefetch abort: reading hwpte addr(%08lx)\n",addr);
				hpte = readl(hw_pte(pte));
                                printk(  "prefetch abort: read hwpte(%08lx) for addr(%08lx)\n", hpte,addr);
				if (hpte && (!(hpte & 0x2))) {
					/* hw pte has bit set, but invalid. seems planted by us */
                                        printk( "prefetch abort: passed test for hwpte(%08lx) for addr(%08lx), our doing\n", hpte,addr);
					
//                                        if (thumb_mode(regs)){
//                                            printk( "prefetch abort: running in thumb mode, will stop monitoring\n", hpte,addr);
//                                            unmask_hwpte(refc); // done in update_refcount
//                                        }
//                                        else{
                                            /* let user to execute the faulty instr */
                                            printk( "prefetch abort: running in arm mode, will continue monitoring\n");
                                            unmask_hwpte(refc); // done in update_refcount
                                            refc = update_refcount(task,pte,addr,regs);
//                                        }
					
					return;	/* skip Linux's handling */
				}
                                else{
                                   printk( "prefetch abort: did not pass test for hwpte(%08lx) for addr(%08lx), our doing\n", hpte,addr); 
                                }
			}
                        else{
                            printk( "prefetch abort: pte_lookup failed for addr(%08lx)\n",addr);
                        }
		}
	}
#endif
       
        
	if (!inf->fn(addr, ifsr | FSR_LNX_PF, regs))
		return;
       
	printk(KERN_ALERT "Unhandled prefetch abort: %s (0x%03x) at 0x%08lx\n",
		inf->name, ifsr, addr);

	info.si_signo = inf->sig;
	info.si_errno = 0;
	info.si_code  = inf->code;
	info.si_addr  = (void __user *)addr;
	arm_notify_die("", regs, &info, ifsr, 0);
}

#ifndef CONFIG_ARM_LPAE
static int __init exceptions_init(void)
{
	if (cpu_architecture() >= CPU_ARCH_ARMv6) {
		hook_fault_code(4, do_translation_fault, SIGSEGV, SEGV_MAPERR,
				"I-cache maintenance fault");
	}

	if (cpu_architecture() >= CPU_ARCH_ARMv7) {
		/*
		 * TODO: Access flag faults introduced in ARMv6K.
		 * Runtime check for 'K' extension is needed
		 */
		hook_fault_code(3, do_bad, SIGSEGV, SEGV_MAPERR,
				"section access flag fault");
		hook_fault_code(6, do_bad, SIGSEGV, SEGV_MAPERR,
				"section access flag fault");
	}

	return 0;
}

arch_initcall(exceptions_init);
#endif
