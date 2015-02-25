#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <ctype.h>
 
#ifndef HEXDUMP_COLS
#define HEXDUMP_COLS 8
#endif

int *temp = NULL;

#define TREE_SIZE 102400
struct node
{
	int key_value;
	struct node *left;
	struct node *right;
};
struct node *root = NULL;

#define MAX_ARRAY_LEN 10240
struct linklist {
        int dummy[MAX_ARRAY_LEN];
        struct linklist* next;
};
struct linklist* testLinkList1 = NULL;
struct linklist* testLinkList2 = NULL;

void insert(int key, struct node *leaf)
{
    if( leaf == NULL )
    {
        leaf = (struct node*) malloc( sizeof( struct node ) );
        (leaf)->key_value = key;
        /* initialize the children to null */
        (leaf)->left = 0;    
        (leaf)->right = 0;  
    }
    else if(key < (leaf)->key_value)
    {
        insert( key, (leaf)->left );
    }
    else if(key >= (leaf)->key_value)
    {
        insert( key, (leaf)->right );
    }
}

struct node *search(int key, struct node *leaf)
{
  if( leaf != 0 )
  {
      if(key==leaf->key_value)
      {
          return leaf;
      }
      else if(key<leaf->key_value)
      {
          return search(key, leaf->left);
      }
      else
      {
          return search(key, leaf->right);
      }
  }
  else return NULL;
}

void test_binary_tree() {
	int i;
	for (i = 0; i < TREE_SIZE; i++) {
		insert(i, root);
	} 
}

void search_tree() {
	int i;
	for (i = 0; i < TREE_SIZE; i++) {
		search(i, root);
	} 
}

void traverse_heap(){
        int i;
        int temp;
        struct linklist* p = testLinkList1;
        struct linklist* q = testLinkList2;
        while (p != NULL) {
                for (i = 0; i < MAX_ARRAY_LEN; i++) {
                        temp = p->dummy[i];
			p->dummy[i] = i + 1;
                }
                p = p->next;
        }
        while (q != NULL) {
                for (i = 0; i < MAX_ARRAY_LEN; i++) {
                        temp = q->dummy[i];
			q->dummy[i] = i + 1;
                }
                q = q->next;
        }
}

void test_heap() {
	int i;
        struct linklist* testLinkList1 = (struct linklist*)malloc(sizeof(struct linklist));
        struct linklist* testLinkList2 = (struct linklist*)malloc(sizeof(struct linklist));
        for (i = 0; i < MAX_ARRAY_LEN; i++) {
                testLinkList1->dummy[i] = i;
        }
        testLinkList1->next = (struct linklist*)malloc(sizeof(struct linklist));
        for (i = 0; i < MAX_ARRAY_LEN; i++) {
                testLinkList1->next->dummy[i] = i;
        }
        testLinkList1->next->next = (struct linklist*)malloc(sizeof(struct linklist));
        for (i = 0; i < MAX_ARRAY_LEN; i++) {
                testLinkList1->next->next->dummy[i] = i;
        }
        testLinkList1->next->next->next = NULL;

        for (i = 0; i < MAX_ARRAY_LEN; i++) {
                testLinkList2->dummy[i] = i;
        }
        testLinkList2->next = (struct linklist*)malloc(sizeof(struct linklist));
        for (i = 0; i < MAX_ARRAY_LEN; i++) {
                testLinkList2->next->dummy[i] = i;
        }
        testLinkList2->next->next = (struct linklist*)malloc(sizeof(struct linklist));
        for (i = 0; i < MAX_ARRAY_LEN; i++) {
                testLinkList2->next->next->dummy[i] = i;
        }
        testLinkList2->next->next->next = NULL;
}
 
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

#define SEC 1
	for (i = 0; i < 20; i++) {
		printf("will read in %d sec...\n", SEC);
		sleep(SEC);
                if (i > 4) {
			printf("temp = %d\n", *temp);
			//printf("Start traversing heap: round %d\n", i);
                        //traverse_heap();
			//printf("Start searching binary tree: round %d\n", i);
			//search_tree();
                }
		val = *(unsigned long *)(p);
		printf("read once. %d\n", i);
	}
}

int main()
{
	char mesg[] = "HelloWorld";

	//test_heap();
	//test_binary_tree();
	temp = (int*)malloc(sizeof(int));
	*temp = 1;
	test_mmap();
	//free(temp);

	/*We need time to check process stats*/
	sleep(100);
	//printf("%s\n", mesg);
//	hexdump((unsigned int *)0x8600, 128);
	printf("bye!\n");
	return 0;
}
