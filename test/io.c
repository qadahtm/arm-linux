#include<stdio.h>
#include<unistd.h>
/* tqadah: */
#include <sched.h>
#include <linux/sched.h>

/* an IO bound task */
int main(int argc, char **argv)
{
	volatile unsigned long k = 0;		   
	int rpolicy = 6;
        struct sched_param sp = { };
        sched_setscheduler(getpid(), rpolicy,&sp);	

	printf("starting an IO-bound task\n");
	while (1) {
		sleep(1);
		
		printf("wake up...\n");

		/* do a short CPU spinning */
		for (k = 0; k < 0xfff; k++)
			;
		
	}
}
