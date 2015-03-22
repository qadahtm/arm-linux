/* Red Black Tree - myrbtree.h
 * Yiyang Chang
 * chang256@purdue.edu
 * Ref 1: http://en.wikipedia.org/wiki/Red%E2%80%93black_tree
 * Ref 2: http://web.mit.edu/~emin/www.old/source_code/red_black_tree/index.html
 * */

#ifndef MYRBTREE_H
#define MYRBTREE_H

typedef struct red_blk_node {
	void* key; // the key should be casted to struct sched_entity*
	struct red_blk_node* left_child;
	struct red_blk_node* right_child;
	struct red_blk_node* parent;
	int red; // red: 1, black: 0
} red_blk_node;

typedef struct red_blk_tree {
	int (*compare) (const void* a, const void* b);
	void (*destroy_key) (void* a);
	void (*print_key) (const void* a);
	red_blk_node* root;
	red_blk_node* nil; // sentinel node
} red_blk_tree;

#endif