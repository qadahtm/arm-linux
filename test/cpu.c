#include<stdio.h>
/* tqadah: */
#include <sched.h>
#include <linux/sched.h>

/* a CPU bound task */
int main(int argc, char **argv)
{
	volatile unsigned long k = 0;		  
	int rpolicy = 6;
    	struct sched_param sp = { };
        sched_setscheduler(getpid(), rpolicy,&sp); 
	
	printf("starting a CPU-bound task\n");
	while (1) {
		/* report the progress once a while */
		if ((k++ & 0xffffff) == 0)
			printf("k = %ld\n", k);
	}
}

