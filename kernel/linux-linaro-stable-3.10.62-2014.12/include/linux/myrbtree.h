/*
  Red Black Trees
  (C) 1999  Andrea Arcangeli <andrea@suse.de>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

  linux/include/linux/rbtree.h

  To use rbtrees you'll have to implement your own insert and search cores.
  This will avoid us to use callbacks and to drop drammatically performances.
  I know it's not the cleaner way,  but in C (not in C++) to get
  performances and genericity...

  See Documentation/rbtree.txt for documentation and samples.
*/

#ifndef	_LINUX_MYRBTREE_H
#define	_LINUX_MYRBTREE_H

#include <linux/kernel.h>
#include <linux/stddef.h>

struct my_rb_node {
	unsigned long  __rb_parent_color;
	struct my_rb_node *rb_right;
	struct my_rb_node *rb_left;
} __attribute__((aligned(sizeof(long))));
    /* The alignment might seem pointless, but allegedly CRIS needs it */

struct my_rb_root {
	struct my_rb_node *my_rb_node;
};

#if 1 // Yiyang: comment out due to symbol name conflicts

#define my_rb_parent(r)   ((struct my_rb_node *)((r)->__rb_parent_color & ~3))

#define MY_RB_ROOT	(struct my_rb_root) { NULL, }
#define	my_rb_entry(ptr, type, member) container_of(ptr, type, member)

#define MY_RB_EMPTY_ROOT(root)  ((root)->rb_node == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbree */
#define MY_RB_EMPTY_NODE(node)  \
	((node)->__rb_parent_color == (unsigned long)(node))
#define MY_RB_CLEAR_NODE(node)  \
	((node)->__rb_parent_color = (unsigned long)(node))


extern void my_rb_insert_color(struct my_rb_node *, struct my_rb_root *);
extern void my_rb_erase(struct my_rb_node *, struct my_rb_root *);


/* Find logical next and previous nodes in a tree */
extern struct my_rb_node *my_rb_next(const struct my_rb_node *);
extern struct my_rb_node *my_rb_prev(const struct my_rb_node *);
extern struct my_rb_node *my_rb_first(const struct my_rb_root *);
extern struct my_rb_node *my_rb_last(const struct my_rb_root *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
extern void my_rb_replace_node(struct my_rb_node *victim, struct my_rb_node *new, 
			    struct my_rb_root *root);

static inline void my_rb_link_node(struct my_rb_node * node, struct my_rb_node * parent,
				struct my_rb_node ** rb_link)
{
	node->__rb_parent_color = (unsigned long)parent;
	node->rb_left = node->rb_right = NULL;

	*rb_link = node;
}

#endif
#endif	/* _LINUX_MYRBTREE_H */
