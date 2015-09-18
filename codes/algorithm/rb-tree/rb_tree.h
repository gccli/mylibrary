#ifndef _RB_TREE_H__
#define _RB_TREE_H__

#include <stdlib.h>
#include <stddef.h>

#define container_of(ptr, type, member) ({                              \
            const typeof( ((type *)0)->member ) *__mptr = (ptr);        \
            (type *)( (char *)__mptr - offsetof(type,member) );         \
        })

typedef enum
{
    RED = 0,
    BLACK = 1
} rb_color_t;

typedef struct _rb_node
{
    struct _rb_node *p;           // parent
    struct _rb_node *left;
    struct _rb_node *right;
    rb_color_t       color;
} rb_node_t;

typedef struct _rb_tree
{
    rb_node_t *root;
    int (*get_key_callback)(rb_node_t *);
} rb_tree_t;

extern rb_node_t rb_nil;

#define p_of(n)  (n)->p
#define pp_of(n) p_of((n))->p
#define rb_entry(ptr, type, member) container_of(ptr, type, member)

static inline int rb_height(rb_node_t *n)
{
    if (n == &rb_nil)
        return 0;

    int h_left = rb_height(n->left);
    int h_right = rb_height(n->right);
    return h_left > h_right ? h_left+1 : h_right + 1;
}

static inline rb_node_t *rb_minimum(rb_node_t *x)
{
    while (x->left != &rb_nil)
        x = x->left;
    return x;
}

static inline rb_node_t *rb_maximum(rb_node_t *x)
{
    while (x->right != &rb_nil)
        x = x->right;
    return x;
}

int rb_insert(rb_tree_t *, rb_node_t *);
rb_node_t *rb_delete(rb_tree_t *, rb_node_t *);
rb_node_t *rb_search(rb_tree_t *, int);
rb_node_t *rb_first(rb_tree_t *);
rb_node_t *rb_next(rb_node_t *);

#endif
