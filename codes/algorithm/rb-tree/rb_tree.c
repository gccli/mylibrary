#include "rb_tree.h"
#include <errno.h>

rb_node_t rb_nil = {&rb_nil, 0, 0, BLACK};

static void left_rotate(rb_tree_t *tree, rb_node_t *x)
{
    rb_node_t *y = x->right;
    x->right = y->left;
    if (y->left != &rb_nil)
        y->left->p = x;

    y->p = x->p;
    if (p_of(x) == &rb_nil)
        tree->root = y;
    else if (x == p_of(x)->left)
        p_of(x)->left = y;
    else
        p_of(x)->right = y;

    y->left = x;
    x->p = y;
}

static void right_rotate(rb_tree_t *tree, rb_node_t *y)
{
    rb_node_t *x = y->left;
    y->left = x->right;
    if (x->right != &rb_nil)
        x->right->p = y;

    x->p = y->p;
    if (p_of(y) == &rb_nil)
        tree->root = x;
    else if (y == p_of(y)->left)
        p_of(y)->left = x;
    else
        p_of(y)->right = x;

    x->right = y;
    y->p = x;
}

static void trans_plant(rb_tree_t *tree, rb_node_t *u, rb_node_t *v)
{
    if (p_of(u) == &rb_nil)
        tree->root = v;
    else if (p_of(u)->left == u) {
        p_of(u)->left = v;
    } else {
        p_of(u)->right = v;
    }
    p_of(v) = p_of(u);
}

static void rb_insert_fixup(rb_tree_t *tree, rb_node_t *z)
{
    rb_node_t *y;

    while(p_of(z)->color == RED) {
        if (p_of(z) == pp_of(z)->left) {
            y = pp_of(z)->right;        // uncle
            if (y->color == RED) {
                // case 1: uncle is red
                p_of(z)->color = BLACK;
                y->color = BLACK;
                pp_of(z)->color = RED;
                z = pp_of(z);
            } else {
                if (z == p_of(z)->right) {
                    // case2: uncle is black and z is right child
                    z = p_of(z);
                    left_rotate(tree, z);
                }
                // case3: uncle is black and z is left child
                p_of(z)->color = BLACK;
                pp_of(z)->color = RED;
                right_rotate(tree, pp_of(z));
            }

        } else if (p_of(z) == pp_of(z)->right) {
            y = pp_of(z)->left;
            if (y->color == RED) {
                p_of(z)->color = BLACK;
                y->color = BLACK;
                pp_of(z)->color = RED;
                z = pp_of(z);
            } else {
                if (z == p_of(z)->left) {
                    z = p_of(z);
                    right_rotate(tree, z);
                }

                p_of(z)->color = BLACK;
                pp_of(z)->color = RED;
                left_rotate(tree, pp_of(z));
            }

        }
    }
    tree->root->color = BLACK;
}

static void rb_delete_fixup(rb_tree_t *tree, rb_node_t *x)
{
    rb_node_t *y;

    while(x != tree->root && x->color == BLACK) {
        if (x == p_of(x)->left) {
            y = p_of(x)->right;        // brothers
            if (y->color == RED) {
                // case 1:  is red
                y->color = BLACK;
                p_of(x)->color = RED;
                left_rotate(tree, p_of(x));
                y = p_of(x)->right;
            }
            if (y->left->color == BLACK && y->right->color == BLACK) {
                y->color = RED;
                x = p_of(x);
            } else {
                if (y->right->color == BLACK) {
                    y->left->color = BLACK;
                    y->color = RED;
                    right_rotate(tree, y);
                    y = p_of(x)->right;
                }
                // case3: uncle is black and z is left child
                y->color = p_of(x)->color;
                p_of(x)->color = BLACK;
                y->right->color = BLACK;
                left_rotate(tree, p_of(x));
                x = tree->root;
            }
        } else if (x == p_of(x)->right) {
            y = p_of(x)->left;        // brothers
            if (y->color == RED) {
                // case 1:  is red
                y->color = BLACK;
                p_of(x)->color = RED;
                right_rotate(tree, p_of(x));
                y = p_of(x)->left;
            }
            if (y->right->color == BLACK && y->left->color == BLACK) {
                y->color = RED;
                x = p_of(x);
            } else {
                if (y->left->color == BLACK) {
                    y->right->color = BLACK;
                    y->color = RED;
                    left_rotate(tree, y);
                    y = p_of(x)->left;
                }
                // case3: uncle is black and z is left child
                y->color = p_of(x)->color;
                p_of(x)->color = BLACK;
                y->left->color = BLACK;
                right_rotate(tree, p_of(x));
                x = tree->root;
            }
        }
    }
    x->color = BLACK;
}

int rb_insert(rb_tree_t *tree, rb_node_t *z)
{
    rb_node_t *x = tree->root;
    rb_node_t *y = &rb_nil;

    int x_key, y_key, z_key;
    z_key = tree->get_key_callback(z);
    while(x != &rb_nil) {
        y = x;
        x_key = tree->get_key_callback(x);
        if (x_key < z_key)
            x = x->right;
        else if (x_key > z_key)
            x = x->left;
        else
            return EEXIST;
    }

    z->p = y;
    y_key = tree->get_key_callback(y);
    if (y == &rb_nil) {
        tree->root = z;
    } else if (y_key > z_key) {
        y->left = z;
    } else {
        y->right = z;
    }

    z->left = z->right = &rb_nil;
    z->color = RED;

    rb_insert_fixup(tree, z);
    return 0;
}


rb_node_t *rb_delete(rb_tree_t *tree, rb_node_t *z)
{
    rb_node_t *x, *y = z;
    rb_color_t y_orig_color = z->color;

    if (z->left == &rb_nil) {
        x = z->right;
        trans_plant(tree, z, z->right);
    } else if (z->right == &rb_nil) {
        x = z->left;
        trans_plant(tree, z, z->left);
    } else {
        y = rb_minimum(z->right);
        y_orig_color = y->color;
        x = y->right;
        if (p_of(y) == z) {
            p_of(x) = y;
        } else {
            trans_plant(tree, y, y->right);
            y->right = z->right;
            p_of(y->right) = y;
        }
        trans_plant(tree, z, y);
        y->left = z->left;
        y->left->p = y;
        y->color = z->color;
    }
    if (y_orig_color == BLACK) {
        rb_delete_fixup(tree, x);
    }

    return y;
}

rb_node_t *rb_search(rb_tree_t *tree, int key)
{
    int p_key;
    rb_node_t *p = tree->root;
    while(p != &rb_nil) {
        p_key = tree->get_key_callback(p);
        if (key == p_key)
            return p;
        else if (key < p_key)
            p = p->left;
        else
            p = p->right;
    }

    return NULL;
}

rb_node_t *rb_first(rb_tree_t *tree)
{
    rb_node_t *n = tree->root;
    if (n == &rb_nil)
        return NULL;

    while(n->left != &rb_nil)
        n = n->left;

    return n;
}

rb_node_t *rb_next(rb_node_t *node)
{
    rb_node_t *p = NULL;
    if (node->right != &rb_nil) {
        node = node->right;
        while (node->left != &rb_nil)
            node = node->left;
        return node;
    }

    while((p = p_of(node)) != &rb_nil && p->right == node)
        node = p;
    if (p == &rb_nil)
        p = NULL;

    return p;
}
