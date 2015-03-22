/* Red Black Tree - myrbtree.c
 * Yiyang Chang
 * chang256@purdue.edu
 * Ref 1: http://en.wikipedia.org/wiki/Red%E2%80%93black_tree
 * Ref 2: http://web.mit.edu/~emin/www.old/source_code/red_black_tree/index.html
 * */

//#include <stdio.h>
//#include <stdlib.h>
#include "myrbtree.h"

#include <linux/printk.h>
#include <linux/slab.h>
#include <asm/bug.h>

/* normal_BST_insert(red_blk_tree*, red_blk_node*) is a helper function 
 * called by red_blk_insert(void*, red_blk_tree*) */
static void normal_BST_insert (red_blk_tree* tree, red_blk_node* node) {
	red_blk_node* nil = tree->nil;
	red_blk_node* x;
	red_blk_node* y;

	node->left_child = node->right_child = nil;
	y = tree->root;
	x = tree->root->left_child;

	while (x != nil) {
		y = x;
		if (tree->compare(node->key, x->key) >= 0) {
			x = x->right_child;
		} else {
			x = x->left_child;
		}
	}
	node->parent = y;
	//if ((y == tree->root) || tree->compare(node->key, y->key) < 0)
	//	y->left_child = node;
	//else
	//	y->right_child = node;
	if (y == tree->root)
		y->left_child = y->right_child = node; // a little hack
	else if (tree->compare(node->key, y->key) < 0)
		y->left_child = node;
	else
		y->right_child = node;
}

static red_blk_node* grandparent (red_blk_node* node) {
	return node->parent->parent;
}

static red_blk_node* uncle (red_blk_node* node) {
	red_blk_node* g = grandparent(node);
	if (node->parent == g->left_child)
		return g->right_child;
	else 
		return g->left_child;
}

static red_blk_node* sibling (red_blk_node* node) {
	if (node == node->parent->left_child)
		return node->parent->right_child;
	else
		return node->parent->left_child;
}

/*      Q                           P
 *     / \     Right Rotation      / \ 
 *    P   C    ------------->     A   Q
 *   / \       <-------------        / \ 
 *  A   B      Left Rotation        B   C 
 *                                        */
static void rotate_left(red_blk_node* P, red_blk_node* Q) {
	P->right_child = Q->left_child;
	Q->left_child->parent = P;
	Q->left_child = P;

	//if (P->parent->left_child == P)
	//	P->parent->left_child = Q;
	//else if (P->parent->right_child == P)
	//	P->parent->right_child = Q;
	//else // P is root
	//	;
	
	if (P->parent->left_child == P)
		P->parent->left_child = Q;
	if (P->parent->right_child == P) // not "else if" due to P->parent could be tree->root
		P->parent->right_child = Q;
	//else // P is root
	//	;
	Q->parent = P->parent;
	P->parent = Q;
}

static void rotate_right(red_blk_node* P, red_blk_node* Q) {
	Q->left_child = P->right_child;
	P->right_child->parent = Q;
	P->right_child = Q;

	//if (Q->parent->left_child == Q)
	//	Q->parent->left_child = P;
	//else if (Q->parent->right_child == Q)
	//	Q->parent->right_child = P;
	//else // Q is root
	//	;
	
	if (Q->parent->left_child == Q)
		Q->parent->left_child = P;
	if (Q->parent->right_child == Q) // not "else if" due to P->parent could be tree->root
		Q->parent->right_child = P;
	P->parent = Q->parent;
	Q->parent = P;
}

red_blk_tree* red_blk_create_tree (int (*compare)(const void*, const void*), 
                                   void (*destroy_key)(void*), 
				   void (*print_key)(const void*)) {
	red_blk_tree* newTree;
	red_blk_node* nil; // sentinel node
	red_blk_node* root; // root node
	
	newTree = (red_blk_tree*)kmalloc(sizeof(red_blk_tree), GFP_KERNEL);
	if (newTree == NULL) {
		printk(KERN_EMERG "[ERROR] kmalloc() failed when allocating new tree.\n");
		BUG();
	}
	newTree->compare = compare;
	newTree->destroy_key = destroy_key;
	newTree->print_key = print_key;

	nil = (red_blk_node*)kmalloc(sizeof(red_blk_node), GFP_KERNEL);
	if (nil == NULL) {
		printk(KERN_EMERG "[ERROR] kmalloc() failed when allocating sentinel.\n");
		BUG();
	}
	newTree->nil = nil;
	nil->parent = nil->left_child = nil->right_child = nil;
	nil->red = 0;
	nil->key = NULL;

	root = (red_blk_node*)kmalloc(sizeof(red_blk_node), GFP_KERNEL);
	if (root == NULL) {
		printk(KERN_EMERG "[ERROR] kmalloc() failed when allocating root.\n");
		BUG();
	}
	newTree->root = root;
	root->parent = root->left_child = root->right_child = nil;
	root->red = 0;
	root->key = NULL;

	return newTree;
}

red_blk_node* red_blk_search (void* key, red_blk_tree* tree) {
	red_blk_node* current_node = tree->root->left_child;
	red_blk_node* nil = tree->nil;

	if (current_node == nil)
		return nil;
	while ((current_node != nil) && (0 != tree->compare(key, current_node->key))) {
		/* search left child */
		if (tree->compare(key, current_node->key) < 0)
			current_node = current_node->left_child;
		/* search right child */
		else
			current_node = current_node->right_child;
	} 

	return current_node;
}

/* case 5: N's parent (P) is red but N's uncle is black. EITHER N is the left
 * child of P, P is the left child of N's grandparent (G), OR N is the right
 * child of P, P is the right child of G*/
static void insert_case5 (red_blk_node* node, red_blk_tree* tree) {
	red_blk_node* g = grandparent(node);

	node->parent->red = 0;
	g->red = 1;
	if (node == node->parent->left_child)
		rotate_right(node->parent, g);
	else
		rotate_left(g, node->parent);
}

/* case 4: N's parent (P) is red but N's uncle is black. EITHER N is the right
 * child of P, P is the left child of N's grandparent (G), OR N is the left
 * child of P, P is the right child of G */
static void insert_case4 (red_blk_node* node, red_blk_tree* tree) {
	red_blk_node* g = grandparent(node);
	if ((node == node->parent->right_child) && (node->parent == g->left_child)) {
		rotate_left(node->parent, node);
		node = node->left_child;
	} else if ((node == node->parent->left_child) && (node->parent == g->right_child)) {
		rotate_right(node, node->parent);
		node = node->right_child;
	}
	insert_case5(node, tree);
}

/* case 3: N's parent and uncle are red */
static void insert_case1(red_blk_node*, red_blk_tree*);
static void insert_case3 (red_blk_node* node, red_blk_tree* tree) {
	red_blk_node* u = uncle(node);
	red_blk_node* g;

	if (u->red == 1) {
		node->parent->red = 0; // paint parent to black
		u->red = 0; // paint uncle to black
		g = grandparent(node);
		g->red = 1; // paint grandparent to red
		insert_case1(g, tree);
	} else
		insert_case4(node, tree);
}

/* case 2: N's parent is black */
static void insert_case2 (red_blk_node* node, red_blk_tree* tree) {
	if (node->parent->red == 0)
		return;
	else 
		insert_case3(node, tree);
}

/* case 1: N is the root node */
static void insert_case1 (red_blk_node* node, red_blk_tree* tree) {
	if (node->parent == tree->root)
		node->red = 0; // the root has to be black
	else
		insert_case2(node, tree);
}

red_blk_node* red_blk_insert (void* key, red_blk_tree* tree) {
	red_blk_node* newNode = (red_blk_node*)kmalloc(sizeof(red_blk_node), GFP_KERNEL);
	red_blk_node* nil = tree->nil;

	if (newNode == NULL) {
		printk(KERN_EMERG "[ERROR] kmalloc() failed when allocating newNode.\n");
		BUG();
	}
	newNode->key = key;
	newNode->red = 1;
	newNode->left_child = nil;
	newNode->right_child = nil;
	newNode->parent = nil;

	normal_BST_insert(tree, newNode);
	insert_case1(newNode, tree);
	//tree->root->left_child->red = 0; // the root->left_child is the real root
	return newNode;
}


/* case 6: S is black, S's right child is red */
static void delete_case6 (red_blk_node* node, red_blk_tree* tree) {
	red_blk_node* s = sibling(node);
	
	s->red = node->parent->red;
	node->parent->red = 0;

	if (node == node->parent->left_child) {
		s->right_child->red = 0;
		rotate_left(node->parent, s);
	} else {
		s->left_child->red = 0;
		rotate_right(s, node->parent);
	}
}

/* case 5: S is black, S's left child is red, S's right child is black */
static void delete_case5 (red_blk_node* node, red_blk_tree* tree) {
	red_blk_node* s = sibling(node);

	if (s->red == 0) { // the test statement is trivial due to case 2
		if ((node == node->parent->left_child) &&
		    (s->right_child->red == 0) &&
		    (s->left_child->red == 1)) { // trivial test due to cases 2-4
			s->red = 1;
			s->left_child->red = 0;
			rotate_right(s->left_child, s);
		} else if ((node == node->parent->right_child) &&
		           (s->left_child->red == 0) &&
			   (s->right_child->red == 1)) { // trivial test due to cases 2-4
			s->red = 1;
			s->right_child->red = 0;
			rotate_left(s, s->right_child);
		}
	}
	delete_case6(node, tree);
}

/* case 4: S and S's children are black, but P is red, we simply exchange the
 * colors of S and P */
static void delete_case4 (red_blk_node* node, red_blk_tree* tree) {
	red_blk_node* s = sibling(node);

	if ((node->parent->red == 1) && 
	    (s->red == 0) &&
	    (s->left_child->red == 0) &&
	    (s->right_child->red == 0)) {
		s->red = 1;
		node->parent->red = 0;
	} else
		delete_case5(node, tree);
}

/* case 3: N's new parent (P), S, and S's children are black, we simply repaint
 * S red */
static void delete_case1 (red_blk_node*, red_blk_tree*);
static void delete_case3 (red_blk_node* node, red_blk_tree* tree) {
	red_blk_node* s = sibling(node);

	if ((node->parent->red == 0) && 
	    (s->red == 0) &&
	    (s->left_child->red == 0) &&
	    (s->right_child->red == 0)) {
		s->red = 1;
		delete_case1(node->parent, tree);
	} else
		delete_case4(node, tree);
}

/* case 2: N's sibling (S) is red. */
static void delete_case2 (red_blk_node* node, red_blk_tree* tree) {
	red_blk_node* s = sibling(node);

	if (s->red == 1) {
		node->parent->red = 1;
		s->red = 0;
		if (node == node->parent->left_child)
			rotate_left(node->parent, s);
		else
			rotate_right(s, node->parent);
	}
	delete_case3(node, tree);
}

/* case 1: N is the new root. */
static void delete_case1 (red_blk_node* node, red_blk_tree* tree) {
	//if (node->parent != tree->nil)
	if (node->parent != tree->root)
		delete_case2(node, tree);
}

/* Helper of red_blk_delete_node(red_blk_tree* tree, red_blk_node* node) */
static void delete_one_child (red_blk_node* node, red_blk_tree* tree) {
	red_blk_node* nil = tree->nil;

	red_blk_node* child = (node->right_child == nil) ? node->left_child : node->right_child;

	/* substitue child into node */
	node->key = child->key;
	//node->red = child->red;
	child->parent = node->parent;
	if (node->parent->left_child == node)
		node->parent->left_child = child;
	else
		node->parent->right_child = child;

	if (node->red == 0) {
		if (child->red == 1)
			child->red = 0;
		else
			delete_case1(child, tree);
	}
	kfree(node);
}

void red_blk_delete_node (red_blk_tree* tree, red_blk_node* node) {
	red_blk_node* nil = tree->nil;
	red_blk_node* suc = NULL;

	/* node has 2 non-leaf child */
	if (node->left_child != nil && node->right_child != nil) {
		suc = red_blk_find_predecessor(tree, node);
		if (suc == nil) {
			printk(KERN_EMERG "[ERROR] No successor found!\n");
			BUG();
		}
		node->key = suc->key;
		//if (suc->left_child != nil) {
		//	if (suc->parent->left_child == suc)
		//		suc->parent->left_child = suc->left_child;
		//	else
		//		suc->parent->right_child = suc->left_child;
		//	suc->left_child->parent = suc->parent;
		//} else if (suc->right_child != nil) {
		//	if (suc->parent->left_child == suc)
		//		suc->parent->left_child = suc->right_child;
		//	else
		//		suc->parent->right_child = suc->right_child;
		//	suc->right_child->parent = suc->parent;
		//} else {
		//	fprintf(stderr, "[ERROR] Should not fall here!\n");
		//	abort();
		//}
		delete_one_child(suc, tree);
	} else {
		delete_one_child(node, tree);
	}
}

static void tree_destroy_helper (red_blk_tree* tree, red_blk_node* root) {
	if (root != tree->nil) {
		tree_destroy_helper(tree, root->left_child);
		tree_destroy_helper(tree, root->right_child);
		kfree(root->key);
		kfree(root);
	}
}

void red_blk_destroy_tree (red_blk_tree* tree) {
	tree_destroy_helper(tree, tree->root->left_child);
	kfree(tree->root);
	kfree(tree->nil);
	kfree(tree);
}

red_blk_node* red_blk_find_predecessor (red_blk_tree* tree, red_blk_node* node) {
	red_blk_node* target = node->left_child;
	red_blk_node* nil = tree->nil;

	if (target != nil) { // find the largest child of the left subtree
		while (target->right_child != nil) {
			target = target->right_child;
		}
		return target;
	} else { // find the closest ancestor (target) whose right-child is my ancestor (node)
		target = node->parent;
		while (node == target->left_child) {
			if (target == tree->root) return nil;
			node = target;
			target = node->parent;
		}
		return target;
	}
}

red_blk_node* red_blk_find_successor (red_blk_tree* tree, red_blk_node* node) {
	red_blk_node* target = node->right_child;
	red_blk_node* nil = tree->nil;

	if (target != nil) { // find the smallest child of the right subtree
		while (target->left_child != nil) {
			target = target->left_child;
		}
		return target;
	} else {
		target = node->parent;
		while (node == target->right_child) {
			if (target == tree->root) return nil;
			node = target;
			target = node->parent;
		}
		return target;
	}
}

red_blk_node* red_blk_find_leftmost (red_blk_tree* tree) {
	red_blk_node* nil = tree->nil;
	//red_blk_node* root = tree->root;
	red_blk_node* target = tree->root->left_child;

	while (target->left_child != nil)
		target = target->left_child;
	return target;

	//if (root == nil) return nil;

	//if (nil != (target = root->left_child)) {
	//	while (target != nil)
	//		target = target->left_child;
	//	return target;
	//} else
	//	return root;
}

red_blk_node* red_blk_find_rightmost (red_blk_tree* tree) {
	red_blk_node* nil = tree->nil;
	red_blk_node* target = tree->root->left_child;

	while (target->right_child != nil)
		target = target->right_child;
	return target;
}

int red_blk_is_empty(red_blk_tree* tree) {
	if (tree->root == tree->nil) return 1;
	else return 0;
}

void red_blk_inorder_tree_print(red_blk_tree* tree, red_blk_node* root) {
	red_blk_node* nil = tree->nil;
	if (root != nil) {
		red_blk_inorder_tree_print(tree, root->left_child);
		printk("key = ");
		tree->print_key(root->key);
		printk(" \tleft->key = ");
		if (root->left_child == nil)
			printk("N");
		else
			tree->print_key(root->left_child->key);
		printk(" \tright->key = ");
		if (root->right_child == nil)
			printk("N");
		else
			tree->print_key(root->right_child->key);
		printk(" \tparent->key = ");
		if (root->parent == tree->root)
			printk("N");
		else
			tree->print_key(root->parent->key);
		printk(" \tred = %i\n", root->red);
		red_blk_inorder_tree_print(tree, root->right_child);
	}
}
