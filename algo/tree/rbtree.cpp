#include "rbtree.h"

rb_node_t sentry;

rb_node_t *RBTree::Minimum(rb_node_t *x)
{
    while (x->left != &sentry)
        x = x->left;
    return x;
}
rb_node_t *RBTree::Maximum(rb_node_t *x)
{
    while (x->right != &sentry)
        x = x->right;
    return x;
}


void RBTree::TransPlant(rb_node_t *u, rb_node_t *v)
{
    if (p_of(u) == &sentry)
        this->root = v;
    else if (p_of(u)->left == u) {
        p_of(u)->left = v;
    } else {
        p_of(u)->right = v;
    }
    p_of(v) = p_of(u);
}

rb_node_t *RBTree::Delete(rb_node_t *z)
{
    rb_node_t *x, *y = z;
    rb_color_t orig_color = z->color;

    if (z->left == &sentry) {
        x = z->right;
        TransPlant(z, z->right);
    } else if (z->right == &sentry) {
        x = z->left;
        TransPlant(z, z->left);
    } else {
        y = Minimum(z->right);
        orig_color = y->color;
        x = y->right;
        if (p_of(y) == z) {
            p_of(x) = y;
        } else {
            TransPlant(y, y->right);
            y->right = z->right;
            p_of(y->right) = y;
        }
        TransPlant(z, y);
        y->left = z->left;
        y->left->p = y;
        y->color = z->color;
    }
    if (orig_color == BLACK) {
        DeleteFixup(x);
    }

    return y;
}

void RBTree::LeftRotate(rb_node_t *x)
{
    rb_node_t *y = x->right;
    x->right = y->left;
    if (y->left != &sentry)
        y->left->p = x;

    y->p = x->p;
    if (p_of(x) == &sentry)
        this->root = y;
    else if (x == p_of(x)->left)
        p_of(x)->left = y;
    else
        p_of(x)->right = y;

    y->left = x;
    x->p = y;
}

void RBTree::RightRotate(rb_node_t *y)
{
    rb_node_t *x = y->left;
    y->left = x->right;
    if (x->right != &sentry)
        x->right->p = y;

    x->p = y->p;
    if (p_of(y) == &sentry)
        this->root = x;
    else if (y == p_of(y)->left)
        p_of(y)->left = x;
    else
        p_of(y)->right = x;

    x->right = y;
    y->p = x;
}

rb_node_t *RBTree::Insert(rb_node_t *z)
{
     rb_node_t *x = this->root;
     rb_node_t *y = &sentry;

     while(x != &sentry) {
         y = x;
         if (x->key < z->key)
             x = x->right;
         else if (x->key > z->key)
             x = x->left;
     }

     z->p = y;
     if (y == &sentry) {
         this->root = z;
     } else if (y->key > z->key) {
         y->left = z;
     } else {
         y->right = z;
     }

     z->left = z->right = &sentry;
     z->color = RED;

     InsertFixup(z);
     return z;
}

void RBTree::InsertFixup(rb_node_t *z)
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
                    LeftRotate(z);
                }
                // case3: uncle is black and z is left child
                p_of(z)->color = BLACK;
                pp_of(z)->color = RED;
                RightRotate(pp_of(z));
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
                    RightRotate(z);
                }

                p_of(z)->color = BLACK;
                pp_of(z)->color = RED;
                LeftRotate(pp_of(z));
            }

        }
    }
    this->root->color = BLACK;
}

void RBTree::DeleteFixup(rb_node_t *x)
{
    rb_node_t *y;

    while(x != this->root && x->color == BLACK) {
        if (x == p_of(x)->left) {
            y = p_of(x)->right;        // brothers
            if (y->color == RED) {
                // case 1:  is red
                y->color = BLACK;
                p_of(x)->color = RED;
                LeftRotate(p_of(x));
                y = p_of(x)->right;
            }
            if (y->left->color == BLACK and y->right->color == BLACK) {
                y->color = RED;
                x = p_of(x);
            } else {
                if (y->right->color == BLACK) {
                    y->left->color = BLACK;
                    y->color = RED;
                    RightRotate(y);
                    y = p_of(x)->right;
                }
                // case3: uncle is black and z is left child
                y->color = p_of(x)->color;
                p_of(x)->color = BLACK;
                y->right->color = BLACK;
                LeftRotate(p_of(x));
                x = this->root;
            }
        } else if (x == p_of(x)->right) {
            y = p_of(x)->left;        // brothers
            if (y->color == RED) {
                // case 1:  is red
                y->color = BLACK;
                p_of(x)->color = RED;
                RightRotate(p_of(x));
                y = p_of(x)->left;
            }
            if (y->right->color == BLACK and y->left->color == BLACK) {
                y->color = RED;
                x = p_of(x);
            } else {
                if (y->left->color == BLACK) {
                    y->right->color = BLACK;
                    y->color = RED;
                    LeftRotate(y);
                    y = p_of(x)->left;
                }
                // case3: uncle is black and z is left child
                y->color = p_of(x)->color;
                p_of(x)->color = BLACK;
                y->left->color = BLACK;
                RightRotate(p_of(x));
                x = this->root;
            }
        }
    }
    x->color = BLACK;
}

rb_node_t *RBTree::Search(int key)
{
  rb_node_t *p = this->root;
  while(p != &sentry) {
    if (key == p->key)
      return p;
    else if (key < p->key)
      p = p->left;
    else
      p = p->right;
  }

  return NULL;
}

rb_node_t *RBTree::First()
{
    rb_node_t *n = this->root;
    if (n == &sentry)
        return NULL;

    while(n->left != &sentry)
        n = n->left;

    return n;
}

rb_node_t *RBTree::Next(rb_node_t *node)
{
    rb_node_t *p = NULL;
    if (node->right != &sentry) {
        node = node->right;
        while (node->left != &sentry)
            node = node->left;
        return node;
    }

    while((p = p_of(node)) != &sentry && p->right == node)
        node = p;
    if (p == &sentry)
        p = NULL;

    return p;
}
