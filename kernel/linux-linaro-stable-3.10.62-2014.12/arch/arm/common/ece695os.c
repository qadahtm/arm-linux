/*
 * Code for ECE695 OS
 * Author: tqadah
 */
#include <linux/slab.h>
#include <linux/ece695os.h>

// process reference count list for bookkeeping.
proc_refc_t *proc_refc_list = NULL;

static void initProcRefList(proc_refc_t *list){
	if (list == NULL){
		list = (proc_refc_t *) kzalloc(sizeof(proc_refc_t),GFP_KERNEL);
	}
	return;
}
