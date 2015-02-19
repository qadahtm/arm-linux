#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gfp.h>
#include <linux/delay.h>
#include <asm/io.h>
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

#include <linux/remoteproc.h>

/* --- for procfs ---- */
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
//#include <asm/uaccess.h>
/* ------------------ */


/*
 * a simplfied version of
    ftp://82.96.64.7/pub/linux/kernel/people/marcelo/linux-2.4/Documentation/DocBook/procfs_example.c
    http://www.stillhq.com/pdfdb/000445/data.pdf

    xzl: code stolen from
    	drivers/platform/x86/toshiba_acpi.c
*/

static struct proc_dir_entry *example_dir;
#define DIR_NAME 	"ece695"

typedef struct procfs_entry_t {
	struct proc_dir_entry * entry;
	const char *name;
} procfs_entry;

u32 mon_vaddr = 0;  /* the vaddr to monitor */

extern void ece695_mask_page_abs(unsigned long vaddr);	/* see fault.c */

static int vaddr_write(struct file *file, const char __user *buffer, size_t count,
			 loff_t *pos)
{
	u32 vaddr;

	if (kstrtou32_from_user(buffer, count, 16, &vaddr) != 0) {
		printk("bad argument.\n");
		return -1;
	}

	mon_vaddr = vaddr;
	printk("write: mon_vaddr %08x\n", mon_vaddr);

	ece695_mask_page_abs(mon_vaddr);

	return count;
}



static int vaddr_read(struct file *f, char __user *p, size_t sz, loff_t *pp)
{
	printk("mon_vaddr %08x\n", mon_vaddr);
	return 0;
}

/* ------------------------------------------------------------- */

procfs_entry entries[] = {
		/* interface changed:
		 * must specify physmem_rw_address before physmem_rw_address */
		{
			.name = "vaddr",
		}
};

static const struct file_operations proc_fops = {
		.read = vaddr_read,
		.write = vaddr_write,
};

static int __init setup_procfs_entries(void)
{
    int rv;
    int i;
    printk("procfs driver init\n");

    /* cr proc dir */
	example_dir = proc_mkdir(DIR_NAME, NULL);
	if(example_dir == NULL) {
	    printk("allocate proc dir failed\n");
		rv = -ENOMEM;
		goto out;
	}

    /* cr file */
    printk("dir created ok\n");

    for (i = 0; i < sizeof(entries)/sizeof(procfs_entry); i++) {
    	entries[i].entry = proc_create(entries[i].name, 0644, example_dir, &proc_fops);
    	if(entries[i].entry == NULL) {
            printk("entry cr %s failed \n", entries[i].name);
            continue;
    	}
    	printk("entry cr %s okay \n", entries[i].name);
    }
    printk("init done\n");
    return 0;       // okay!

out:
	printk("init failed\n");
    return -1;
}

#if 0
void cleanup_module(void)
{
	remove_proc_entry("foo", example_dir);
	remove_proc_entry(MODULE_NAME, NULL);
}
#endif

device_initcall(setup_procfs_entries);

//MODULE_LICENSE("GPL v2");
