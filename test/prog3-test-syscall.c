#include <stdio.h>
#include <unistd.h>

int main(int argc, char ** argv){
	printf("testing a newly added syscall\n");
	pid_t pid = (pid_t) 1000;
	int limit = 40;	
	int ret = syscall(380,pid,limit);
	printf("returned form syscall with ret = %d\n",ret);
	return 0;
}
