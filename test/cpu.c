#include<stdio.h>
/* tqadah: */
#include <sched.h>
#include <linux/sched.h>
#include <unistd.h>

/* a CPU bound task */
int main(int argc, char **argv)
{
	volatile unsigned long k = 0;		  
	int rpolicy = 6;
    	struct sched_param sp = { };
	unsigned int limit = 10;
	int ret = -1;

        sched_setscheduler(getpid(), rpolicy,&sp); 

	ret = syscall(380, getpid(), limit);
	printf("syscall returned %d\n", ret);

	
	printf("starting a CPU-bound task\n");
	while (1) {
		/* report the progress once a while */
		if ((k++ & 0xffffff) == 0)
			;//printf("k = %ld\n", k);
	}
}

