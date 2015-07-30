#include "rbtree.hpp"
#include <string.h>

void RBTree::LeftRotate(BiNode_t *x)
{
    BiNode_t *y = x->right;
    x->right = y->left;
    if (y->left != this->nil)
        y->left->p = x;

    y->p = x->p;
    if (p_of(x) == this->nil)
        this->root = y;
    else if (x == p_of(x)->left)
        p_of(x)->left = y;
    else
        p_of(x)->right = y;

    y->left = x;
    x->p = y;
}

void RBTree::RightRotate(BiNode_t *y)
{
    BiNode_t *x = y->left;
    y->left = x->right;
    if (x->right != this->nil)
        x->right->p = y;

    x->p = y->p;
    if (p_of(y) == this->nil)
        this->root = x;
    else if (y == p_of(y)->left)
        p_of(y)->left = x;
    else
        p_of(y)->right = x;

    x->right = y;
    y->p = x;
}

static char buff[1024];
static long offs;

static void print_tree(BiNode_t *root, BiNode_t *nil, int indent)
{
    BiNode_t *p = root;

    if(p != nil) {
        offs += sprintf(buff+offs, "%d(%s)[", p->key, p->color==RED?"R":"B");
        if (p->left == nil && p->right == nil) {
            offs += sprintf(buff+offs, "]");
            return;
        }
        if (p->left) {
            print_tree(p->left, nil, indent);
            offs += sprintf(buff+offs, ",");
        }

        if (p->right){
            print_tree(p->right, nil, indent);
            offs += sprintf(buff+offs, "]");
        }
    }
}

void RBTree::Print()
{
    int i,j;
    int indent = 0;
    offs = 0;

    char node[32] = {0};
    print_tree(this->root, this->nil, 0);
    for(i=0,j=0; i<offs; ++i) {
        char c = buff[i];
        switch(c) {
        case '[':
            if (buff[i+1] == ',') {
                node[j++] = c;
                node[j++] = ',';
            }
            else if (buff[i+1] != ']') {
                node[j++] = c;
            }

            printf("%*s%s\n", indent, "", node);
            memset(node, 0, sizeof(node));
            j=0;
            indent += 4;
            break;
        case ']':
            indent -= 4;
            if (buff[i-1] != '[')
                printf("%*s%c\n", indent, "", c);
            break;
        case ',':
            break;
        default:
            node[j++] = c;
            break;
        }

    }
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

     z->p = y;
     if (y == this->nil) {
         this->root = z;
     } else if (y->key > z->key) {
         y->left = z;
     } else {
         y->right = z;
     }

     z->left = z->right = this->nil;
     z->color = RED;

     InsertFixup(z);
     return z;
}

void RBTree::InsertFixup(BiNode_t *z)
{
    BiNode_t *y;

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

    while((p = p_of(node)) != this->nil && p->right == node)
        node = p;
    if (p == this->nil)
        p = NULL;

    return p;
}
