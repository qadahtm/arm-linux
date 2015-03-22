/* Red Black Tree - myrbtree.h
 * Yiyang Chang
 * chang256@purdue.edu
 * Ref 1: http://en.wikipedia.org/wiki/Red%E2%80%93black_tree
 * Ref 2: http://web.mit.edu/~emin/www.old/source_code/red_black_tree/index.html
 * */

#ifndef MYRBTREE_H2
#define MYRBTREE_H2

red_blk_tree* red_blk_create_tree (int (*compare)(const void*, const void*), 
                                   void (*destroy_key)(void*), 
				   void (*print_key)(const void*));
red_blk_node* red_blk_search (void*, red_blk_tree*);
red_blk_node* red_blk_search_key (void* , int (*keycompare)(const void*, const void*), red_blk_tree*);
red_blk_node* red_blk_insert (void*, red_blk_tree*);
void red_blk_delete_node (red_blk_tree*, red_blk_node*);
void red_blk_destroy_tree (red_blk_tree*);
red_blk_node* red_blk_find_predecessor (red_blk_tree*, red_blk_node*);
red_blk_node* red_blk_find_successor (red_blk_tree*, red_blk_node*);
red_blk_node* red_blk_find_leftmost (red_blk_tree*);
red_blk_node* red_blk_find_rightmost (red_blk_tree*);
void red_blk_inorder_tree_print (red_blk_tree*, red_blk_node*);
int red_blk_is_empty(red_blk_tree*);

#endif
