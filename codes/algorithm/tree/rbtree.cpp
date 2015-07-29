#include "rbtree.hpp"

void RBTree::LeftRotate(BiNode_t *x)
{
    BiNode_t *y = x->right;
    x->right = y->left;
    if (y->left != this->nil)
        y->left->parent = x;

    y->parent = x->parent;
    if (PARENT(x) == this->nil)
        this->root = y;
    else if (x == PARENT(x)->left)
        PARENT(x)->left = y;
    else
        PARENT(x)->right = y;

    y->left = x;
    x->parent = y;
}

void RBTree::RightRotate(BiNode_t *y)
{
    BiNode_t *x = y->left;
    y->left = x->right;
    if (x->right != this->nil)
        x->right->parent = y;

    x->parent = y->parent;
    if (PARENT(y) == this->nil)
        this->root = x;
    else if (y == PARENT(y)->left)
        PARENT(y)->left = x;
    else
        PARENT(y)->right = x;

    x->right = y;
    y->parent = x;
}

BiNode_t *RBTree::Insert(BiNode_t *z)
{
     BiNode_t *x = this->root;
     BiNode_t *y = this->nil;

     while(x != this->nil) {
         y = x;
         if (x->key < z->key)
             x = x->right;
         else if (x->key > z->key)
             x = x->left;
     }

     z->parent = y;
     if (y == this->nil) {
         this->root = z;
     } else if (y->key > z->key) {
         y->left = z;
     } else {
         y->right = z;
     }

     z->left = this->nil;
     z->right = this->nil;
     z->color = RED;
     InsertFixup(z);
     return z;
 }

 void RBTree::InsertFixup(BiNode_t *z)
 {
     BiNode_t *y;
     BiNode_t *p, *pp; // parent of z and parent of parent of z

     while(PARENT(z)->color == RED) {
         p = PARENT(z);
         pp = PARENT(p);
         if (p == pp->left) {
             y = pp->right;
             if (y->color == RED) {
                 p->color = BLACK;
                 y->color = BLACK;
                 pp->color = RED;
                 z = pp;
             } else if (z == p->right) {
                 z = p;
                 LeftRotate(z);
             }
             p->color = BLACK;
             pp->color = RED;
             RightRotate(pp);
         } else {
             y = pp->left;
             if (y->color == RED) {
                 p->color = BLACK;
                 y->color = BLACK;
                 pp->color = RED;
                 z = pp;
             } else if (z == p->left) {
                 z = p;
                 RightRotate(z);
             }
             p->color = BLACK;
             pp->color = RED;
             LeftRotate(pp);
         }
     }
     this->root->color = BLACK;
}


BiNode_t *RBTree::First()
{
    BiNode_t *n = this->root;
    if (n == this->nil)
        return NULL;

    while(n->left != this->nil)
        n = n->left;

    return n;
}

BiNode_t *RBTree::Next(BiNode_t *node)
{
    BiNode_t *p = NULL;
    if (node->right != this->nil) {
        node = node->right;
        while (node->left != this->nil)
            node = node->left;
        return node;
    }

    while((p = PARENT(node)) != this->nil && p->right == node)
        node = p;
    if (p == this->nil)
        p = NULL;

    return p;
}
