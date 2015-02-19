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
	unsigned int flags = FAULT_FLAG_ALLOW_RETRY | FAULT_FLAG_KILLABLE;

	if (notify_page_fault(regs, fsr))
		return 0;

	tsk = current;
	mm  = tsk->mm;

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
	do_bad_area(addr, fsr, regs);
	return 0;
}

/*
 * This abort handler always returns "fault".
 */
static int
do_bad(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
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

/* Returns the pte* given mm and addr.
 * This is modeled after show_pte().
 */
//static 
pte_t *pte_lookup(struct mm_struct  *mm, int usermode, unsigned long addr)
{
	pgd_t *pgd; pmd_t *pmd; pte_t *pte;
        
        printk("looking up addr = %08lx \n",addr);
	pgd = pgd_offset(mm, addr);

	printk("gets pgd @ %08x\n", (u32)pgd);

	if (!pgd || !pgd_present(*pgd)) {
		printk("bad pgd\n");
		return NULL;
	}

	pmd = pmd_offset(pud_offset(pgd, addr), addr);
	if (!pmd || !pmd_present(*pmd)) {
		printk("bad pmd\n");	// this can happen due to on-demand paging
		return NULL;
	}

	printk("pmd val %08x\n", *pmd);

	pte = pte_offset_map(pmd, addr);
	if (!pte) {
		printk("fail to get pte\n");
		return NULL;
	}

	if (!pte_present(*pte) || (usermode && !pte_present_user(*pte))) {
		printk("bad pte val %08x\n", *pte);  // can happen
		return NULL;
	}

	return pte;
}

/* Mark hardware pte as invalid so that future memory access will trigger
 * exception.
 */
static void mask_hwpte(pte_t *pte)
{
	unsigned long hpte;
	hpte = readl(hw_pte(pte));

	printk("hpte was %08x, going to disable it.\n", (u32)hpte);

	writel(hpte & ~0x3, (void *)hw_pte(pte));

	/* We've made changes to hwpte. Flush the changes */
	clean_dcache_area((void *)(hw_pte(pte)), sizeof(pte_t));   /* right API? */

	/* flush tlb. Necessary. Othwerise future accesses may use stale TLB
	 * for xslat which cannot be trapped. */
	flush_tlb_kernel_page(__phys_to_virt(hpte & PAGE_MASK));
}

/* Mark hardware pte as valid so that future memory access will NOT trigger
 * exception.
 */
static int unmask_hwpte(pte_t *pte)
{
	unsigned long hpte;

	hpte = readl(hw_pte(pte));
	printk("hpte was %08x. going to enable it.\n", (u32)hpte);

	writel(hpte | 0x3, hw_pte(pte));
	clean_dcache_area((void *)(hw_pte(pte)), sizeof(pte_t)); /*bring TLB in sync. no $inv*/
	//flush_tlb_kernel_page(__phys_to_virt(hpte & PAGE_MASK)); /* necessary? */
	return 1;
}

//static struct mm * testmm = 0;
//static int ref= 0;
//static unsigned int saved_instr = 0;
//static unsigned int saved_pc = 0;
//static pte_t *saved_pte = 0; 	/* so that we don't have to lookup again */

void ece695_mask_page_abs(unsigned long vaddr) {}

void ece695_mask_page(struct task_struct *tsk, unsigned long vaddr)
{
//	struct task_struct *tsk;
	struct mm_struct *mm = tsk->mm;
	pte_t * pte;

//	if (!testmm) {
//		printk("didn't see xzltestprog so far. do nothing.\n");
//	}
//	mm = testmm;
//	show_pte(mm, vaddr);
        
        if (tsk && tsk->mm) {
		
		if (strncmp(tsk->comm, "xzltestprog", TASK_COMM_LEN) == 0) {
                    	pte = pte_lookup(mm, 1, vaddr);
                        printk("pte lookup returns %08x\n", (unsigned int)pte);
                        if (pte == NULL) {
                                printk("[ERROR] pte_lookup() failed\n");
                                return;
                        }
                        mask_hwpte(pte);                 
                }
        }

}

/* Replace the next user instruction as a special Undefined instruction, which
 * will trigger exception and thus trap into the kernel again.
 *
 * This allows us to execute the faulty instruction, while letting us to turn
 * the pte back into invalid right after the faulty instruction is executed.
 */
void patch_instr(struct refcount* refc, unsigned long addr, struct mm_struct *mm, struct pt_regs *regs)
{
	/* see do_undefinstr() for a systematic way of doing this */
    
	unsigned long pc  = regs->ARM_pc;
	unsigned int instr;	/* the faulty instr */
	unsigned int undef = BUG_INSTR_VALUE;
	pte_t * pte;
	unsigned long hpte;
        
        /* Read the faulty instruction from pc */
	if (get_user(instr, (u32 __user *)pc) == 0)
		printk("faulty pc %08x, instr %08x\n", pc, instr);

	/*
	 * Make the instruction page r/w so that we can plant the undefinstr.
	 * Originally, the instruction page should be r/o.
	 */
	printk("looking up pc \n");
        pte = pte_lookup(mm, 1, pc);
        
	if (!pte) {
		printk("bug -- why no pte for the faulty pc?\n");
		BUG();
	}
        
        

#if 0	/* debugging */
	if (pte_write(*pte))
		printk("page r/w\n");
	else
		printk("page r/o\n");
#endif

	hpte = readl(hw_pte(pte));

#if 1	/* debugging */
	printk("hpte %08x PTE_EXT_APX %d PTE_EXT_AP1 %d\n", hpte,
			PTE_EXT_APX & hpte, PTE_EXT_AP1 & hpte);
#endif

	set_pte_at(mm, addr, pte, pte_mkwrite(*pte));
	writel(hpte & (~PTE_EXT_APX), hw_pte(pte));

//	flush_cache_all();	/* cannot flush, why? */

	printk(" --- after change --- \n");

#if 0	/* debugging */
	if (pte_write(*pte))
		printk("page r/w\n");
	else
		printk("page r/o\n");
#endif

	hpte = readl(hw_pte(pte));
	printk("hpte %08x PTE_EXT_APX %d\n", hpte, PTE_EXT_APX & hpte);

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
	pc += 4;
        
        refc->saved_pc = pc; 	/* undefinstr handler will need this */
	/* save the next user instruction */
	if (get_user(refc->saved_instr, (u32 __user *)pc) == 0)
		printk("to patch @%08x, saved instr %08x\n", pc, refc->saved_instr);
	else {
		printk("bug -- cannot read instr at %08x\n", pc);
		BUG();
	}

	/* overwrite the next user instruction as an undef instr */
	if (put_user(undef, (u32 __user *)pc) == 0)
		printk("write undef instr ok\n");
	else {
		printk("bug -- cannot write the undef instr. \n");
		BUG();
	}

#if 0
	if (__copy_to_user((u32 __user *)pc, &undef, sizeof(unsigned int)) == 0)
		printk("__copy_to_user okay\n");
	else
		printk("__copy_to_user failed\n");
#endif

//	__flush_icache_all();
	flush_cache_all();	// <- xzl: really needed? XXX

	get_user(instr, (u32 __user *)pc);
	printk("read again: faulty pc %08x, instr %08x\n", pc, instr);
	printk("patch is done\n");
}

/* will be invoked upon undefinstr exception.
 * return 0 on success */
int ece695_restore_saved_instr(struct pt_regs *regs)
{
	unsigned int pc;
	unsigned int instr;
        struct refcount * refc = current->refcount_head;
        
	pc = (void __user *)instruction_pointer(regs);

        printk("look up refc\n");
        
        while (refc != NULL){
            if (refc->saved_pc == pc) break;
            refc = refc->next;
        }
        
        if (refc == NULL){
            printk("cannot find corresponding refc entry\n");
            BUG();
            return -1;
        }
        
        if ((!refc->saved_instr) || (!refc->saved_pc)) {
		printk("no saved instr found -- why?\n");
		return -1;
	}
                
                
//	if ((unsigned int)pc != current->saved_pc)
//		return -1;

	printk("oh this undefinstr is planted by us. \n");
	get_user(instr, (u32 __user *)pc);
	printk("read: faulty pc %08x, instr %08x\n", pc, instr);

	if (put_user(refc->saved_instr, (u32 __user *)pc) == 0)
		printk("restore instr ok -- user good to go\n");
	else {
		printk("bug -- cannot restore instr. \n");
		BUG();
	}

	/* XXX: make the instruction page as r/o for security */

	get_user(instr, (u32 __user *)pc);
	printk("read again: faulty pc %08x, instr %08x\n", pc, instr);

	refc->saved_instr = 0;
	refc->saved_pc = 0;

	if (!refc->pte) {
		printk("bug -- no saved pte?\n");
		BUG();
	}

	/* ready to capture the next user access to the same page */
	mask_hwpte(refc->pte);
	return 0;
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
	unsigned long hpte;

	struct refcount* newref = NULL;
	struct refcount* iter = NULL;
        struct refcount* refc = NULL;
        unsigned int cref =0;

	/* xzl: we have a fault; is the fault caused by us? */
	// (fsr & FSR_FS3_0) == PAGE_TRANSLATION_FAULT) ??
#if 1	/* simple heuristics. you can go fancy of course */
	if (current && current->mm) {
		tsk = current; mm = current->mm;
		if (strncmp(tsk->comm, "xzltestprog", TASK_COMM_LEN) == 0) {
			pte = pte_lookup(mm, 1, addr);
			if (pte) {
				hpte = readl(hw_pte(pte));
				if (hpte && (!(hpte & 0x3))) {
					/* hw pte has bit set, but invalid. seems planted by us */
//					printk("===> Woo-hoo! caught an access. refcount=%d\n", ++ref);
                                        
					/* Yiyang: update refcounts link list in mm_struct */
					if (tsk->refcount_head == NULL) {
						if (NULL == (newref = 
									(struct refcount *)kzalloc(
									sizeof(struct refcount), GFP_KERNEL))) {
							printk(KERN_EMERG "[ERROR] kzalloc() failed. 1\n");
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
						/* search the link list to find the right pte */
						iter = tsk->refcount_head;
						while (iter != NULL) {
//							if (iter->vaddr == addr) {
//								iter->n += 1;
//								break;
//							}
                                                    if (*(iter->pte) == *pte) {
								iter->n += 1;
								break;
                                                    }
						}
						/* didn't find the vaddr in the link list, kzalloc a new one */
						if (iter == NULL) {
							if (NULL == (newref = 
										(struct refcount *)kzalloc(
										sizeof(struct refcount), GFP_KERNEL))) {
								printk(KERN_EMERG "[ERROR] kzalloc() failed. 2\n");
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
                                                    iter->n = (iter->n +1);
                                                    cref = iter->n;
                                                    refc = iter;
                                                }
					}
                                        printk("===> Woo-hoo! caught an access. refcount=%d for addr = %08lx\n", cref,addr);
					#if 0 /* Yiyang: test: walk through the refcount link list */
						printk("[DEBUG] Walk through the refcount link list\n");
						iter = tsk->refcount_head;
						while (iter != NULL) {
							printk("[DEBUG] 0x%x:%u\n", iter->vaddr, iter->n);
							iter = iter->next;
						}
					#endif

					if (cref < 5) {	/* for quick test */
						patch_instr(refc,addr, mm, regs);
						//tsk->saved_pte = pte;
					} else
						printk("refcount large enough. stop monitoring\n");

					/* let user to execute the faulty instr */
					unmask_hwpte(pte);
					return;	/* skip Linux's handling */
				}
			}
		}
	}
#endif

	if (!inf->fn(addr, fsr & ~FSR_LNX_PF, regs)) {
		/* xzl: exception handling okay */
#if 0
		tsk = current;
		mm  = tsk->mm;
		if (strncmp(tsk->comm, "xzltestprog", TASK_COMM_LEN) == 0) {
			testmm = mm; 	// remember it
//			printk("I got a page fault -- fixed \n");
//			show_pte(mm, addr);
		}
#endif
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
