#include "rbtree.hpp"

BiNode_t *RBTree::Insert(BiNode_t *n)
{
     BiNode_t *nil = this->nil;

     BiNode_t *x = this->root;
     BiNode_t *y = x;
     while(x != nil) {
         y = x;
         if (x->key < n->key)
             x = x->right;
         else if (x->key > n->key)
             x = x->left;
     }

     n->parent = y;
     if (y == nil) {
         this->root = n;
     } else if (y->key > n->key) {
         y->left = n;
     } else {
         y->right = n;
     }

     n->left = nil;
     n->right = nil;
     n->color = RED;
     InsertFixup(n);
 }

 void RBTree::InsertFixup(BiNode_t *z)
 {
     BiNode_t *y;
     BiNode_t *p, *pp; // parent of z and parent of parent of z

     while(z->parent->color == RED) {
         p = z->parent;
         pp = p->parent;
         if (p == pp->left) {
             y = pp->right;
             if (y->color == RED) {
                 p->color = BLACK;
                 y->color = BLACK;
                 pp->color = RED;
                 z = pp;
             } else if (z == p->right) {
                 z = p;
                 left_rotate(z);
             }
             p->color = BLACK;
             pp->color = RED;
             right_rotate(pp);
         } else {
             y = pp->left;
             if (y->color == RED) {
                 p->color = BLACK;
                 y->color = BLACK;
                 pp->color = RED;
                 z = pp;
             } else if (z == p->left) {
                 z = p;
                 right_rotate(z);
             }
             p->color = BLACK;
             pp->color = RED;
             left_rotate(pp);
         }
     }
     this->root->color = BLACK;
}
