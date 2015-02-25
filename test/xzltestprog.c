#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include <ctype.h>
 
#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif
 
void hexdump(unsigned int *mem, unsigned int len)
{
	int i;
	for (i = 0; i < len; i ++)
		printf("%08x: %08x\n", (unsigned int)(mem+i), mem[i]);
}

void test_mmap()
{
	int fd;
	int i;
	void *p;
	unsigned long volatile val;

	fd = open("/dev/zero", O_RDWR); 
	if (!fd) {
		perror("test_mmap");
		return;
	}

	p = mmap((void *)0x40000000, 16, PROT_READ | PROT_WRITE, 
		MAP_PRIVATE | MAP_FILE, fd, 0);

	if (!p) {
		perror("test_mmap");
		return;
	}

	printf("mmap p %08x\n", (unsigned long)p);

#define SEC 3 
	for (i = 0; i < 10; i++) {
		printf("will read in %d sec...\n", SEC);
		sleep(SEC);
		val = *(unsigned long *)(p);
		printf("read once. %d\n", i);
	}
}

int main()
{
	char mesg[] = "HelloWorld";
	//sleep(3);
	test_mmap();

	/*We need time to check process stats*/
//	sleep(100);
	//printf("%s\n", mesg);
//	hexdump((unsigned int *)0x8600, 128);
	printf("bye!\n");
	return 0;
}
