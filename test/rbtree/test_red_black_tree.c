/* Red Black Tree - test_red_black_tree.c 
 * Yiyang Chang
 * chang256@purdue.edu
 * Ref 1: http://en.wikipedia.org/wiki/Red%E2%80%93black_tree
 * Ref 2: http://web.mit.edu/~emin/www.old/source_code/red_black_tree/index.html
 * */

#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>

#include"myrbtree.h"

/*  this file has functions to test a red-black tree of integers */

void IntDest(void* a) {
  free((int*)a);
}

int IntComp(const void* a,const void* b) {
  if( *(int*)a > *(int*)b) return(1);
  if( *(int*)a < *(int*)b) return(-1);
  return(0);
}

void IntPrint(const void* a) {
  printf("%i",*(int*)a);
}

int main() {
  int option=0;
  int newKey;
  int* newInt;
  red_blk_node* newNode;
  red_blk_tree* tree;

  tree = red_blk_create_tree(IntComp,IntDest,IntPrint);
  while(option!=9) {
    printf("choose one of the following:\n");
    printf("(1) add to tree\n(2) delete from tree\n(3) query\n");
    printf("(4) find predecessor\n(5) find sucessor\n");
    printf("(6) find leftmost\n(7) find rightmost\n");
    printf("(8) print tree\n(9) quit\n");
    do option=fgetc(stdin); while(-1 != option && isspace(option));
    option-='0';
    switch(option)
      {
      case 1:
	{
	  printf("type key for new node\n");
	  scanf("%i",&newKey);
	  newInt = (int*)malloc(sizeof(int));
	  *newInt = newKey;
	  red_blk_insert(newInt, tree);
	}
	break;
	
      case 2:
	{
	  printf("type key of node to remove\n");
	  scanf("%i",&newKey);
	  if ( tree->nil != ( newNode = red_blk_search(&newKey, tree) ) ) /*assignment*/
	  	red_blk_delete_node(tree, newNode);
	  else 
	  	printf("key not found in tree, no action taken\n");
	}
	break;

      case 3:
	{
	  printf("type key of node to query for\n");
	  scanf("%i",&newKey);
	  if ( tree->nil != ( newNode = red_blk_search(&newKey, tree) ) ) /*assignment*/
	    printf("data found in tree at location %p\n",(void*)newNode);
	  else 
	    printf("data not in tree\n");
	}
	break;
      case 4:
	{
	  printf("type key of node to find predecessor of\n");
	  scanf("%i",&newKey);
	  if ( tree->nil != ( newNode = red_blk_search(&newKey, tree) ) ) { /*assignment*/
	    newNode = red_blk_find_predecessor(tree, newNode);
	    if(tree->nil == newNode) {
	      printf("there is no predecessor for that node (it is a minimum)\n");
	    } else {
	      printf("predecessor has key %i\n",*(int*)newNode->key);
	    }
	  } else {
	    printf("data not in tree\n");
	  }
	}
	break;
      case 5:
	{
	  printf("type key of node to find successor of\n");
	  scanf("%i",&newKey);
	  if ( tree->nil != ( newNode = red_blk_search(&newKey, tree) ) ) { /*assignment*/
	    newNode = red_blk_find_successor(tree, newNode);
	    if(tree->nil == newNode) {
	      printf("there is no successor for that node (it is a maximum)\n");
	    } else {
	      printf("successor has key %i\n",*(int*)newNode->key);
	    }
	  } else {
	    printf("data not in tree\n");
	  }
	}
	break;
      case 6:
	{
	  if ( tree->nil != ( newNode = red_blk_find_leftmost(tree) ) ) { /*assignment*/
	  	printf("Left most key = %d\n", *(int*)newNode->key);
	  } else {
	    printf("tree is empty\n");
	  }
	}
	break;
      case 7:
	{
	  if ( tree->nil != ( newNode = red_blk_find_rightmost(tree) ) ) { /*assignment*/
	  	printf("Right most key = %d\n", *(int*)newNode->key);
	  } else {
	    printf("tree is empty\n");
	  }
	}
	break;
      case 8:
	{
	  red_blk_inorder_tree_print(tree, tree->root->left_child);
	}
	break;
      case 9:
	{
	  red_blk_destroy_tree(tree);
	  return 0;
	}
	break;
      default:
	printf("Invalid input; Please try again.\n");
      }
  }
  return 0;
}
