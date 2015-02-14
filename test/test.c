#include <unistd.h> 
#include <stdio.h>

typedef struct pte_refc  pte_refc_t;
struct pte_refc {
   int refcount;
   //pte_t *pte;
   int *pte;
   pte_refc_t *next_pte;
};

typedef struct proc_refc proc_refc_t;

struct proc_refc
{
    //pid_t pid;	
   int pid; 
   pte_refc_t *pte_list;
   proc_refc_t *next;
};

int main(){ 

//sleep(10000); 
printf("here\n");

proc_refc_t * ref_list = (proc_refc_t *) malloc(sizeof(proc_refc_t));
proc_refc_t *proc = NULL;

ref_list->pid = 0;
ref_list->pte_list = (pte_refc_t *) malloc(sizeof(pte_refc_t));
ref_list->next = NULL;

proc = (proc_refc_t *) malloc(sizeof(proc_refc_t));
proc->pid = 1;
proc->pte_list = (pte_refc_t *) malloc(sizeof(pte_refc_t));
proc->next = NULL;

ref_list->next = proc;

proc = (proc_refc_t *) malloc(sizeof(proc_refc_t));
proc->pid = 2;
proc->pte_list = (pte_refc_t *) malloc(sizeof(pte_refc_t));
proc->next = NULL;

ref_list->next->next = proc;

proc = ref_list;
while (proc != NULL) {
   printf("pid = %d\n", proc->pid);
   proc = proc->next;
}

return 0;

}
