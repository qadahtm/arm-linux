/* tqadah: derived from : http://www.amparo.net/ce155/fork-ex.html */

#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <sys/wait.h>   /* Wait for Process Termination */
#include <stdlib.h>     /* General Utilities */

/* tqadah: */
#include <sched.h>
#include <linux/sched.h>

#define NUM_THREADS 10


int main(int argc, char ** argv){
    pid_t childpid; /* variable to store the child's pid */
    int retval;     /* child process: user-provided return code */
    int status;     /* parent process: child's exit status */

    int policy = SCHED_NORMAL;
    int rpolicy = 6;
    //int rpolicy = SCHED_FIFO;
    int i =0;
    struct sched_param sp = { };
	/* only 1 int variable is needed because each process would have its
       own instance of the variable
       here, 2 int variables are used for clarity */
	
// set scheduler 
	sched_setscheduler(getpid(), rpolicy,&sp);
        
    /* now create new process */
    childpid = fork();
    
    if (childpid >= 0) /* fork succeeded */
    {
        if (childpid == 0) /* fork() returns 0 to the child process */
        {
            printf("CHILD(%d): I am the child process!\n",i);
            printf("CHILD(%d): Here's my PID: %d\n",i, getpid());
            printf("CHILD(%d): My parent's PID is: %d\n",i, getppid());
            printf("CHILD(%d): The value of my copy of childpid is: %d\n",i, childpid);
	    // set scheduler 
	    policy = sched_getscheduler(childpid);
	    printf("CHILD(%d): Sleeping for 1 second... , current sched policy = %d\n",i,policy);
            sleep(1); /* sleep for 1 second */
            printf("CHILD(%d): Goodbye!\n",i);    
        }
        else /* fork() returns new pid to the parent process */
        {
            printf("PARENT: I am the parent process!\n");
            printf("PARENT: Here's my PID: %d\n", getpid());
            printf("PARENT: The value of my copy of childpid is %d\n", childpid);
            printf("PARENT: I will now wait for my child to exit.\n");
            
	    policy = sched_getscheduler(childpid);
	    printf("PARENT: current sched. policy = %d\n",policy);

	    wait(&status); /* wait for child to exit, and store its status */
            printf("PARENT: Child's exit code is: %d\n", WEXITSTATUS(status));
            printf("PARENT: Goodbye!\n");             
            exit(0);  /* parent exits */       
        }
    }
    else /* fork returns -1 on failure */
    {
        perror("fork"); /* display error message */
        exit(0); 
    }
}
