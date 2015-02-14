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
