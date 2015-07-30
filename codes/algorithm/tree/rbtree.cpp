#include "rbtree.h"
#include <string.h>

rb_node_t *RBTree::Minimum(rb_node_t *x)
{
    while (x->left != this->nil)
        x = x->left;
    return x;
}
rb_node_t *RBTree::Maximum(rb_node_t *x)
{
    while (x->right != this->nil)
        x = x->right;
    return x;
}


void RBTree::TransPlant(rb_node_t *u, rb_node_t *v)
{
    if (p_of(u) == this->nil)
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
    int key = z->key;

    rb_node_t *x, *y = z;
    rb_color_t orig_color = z->color;

    if (z->left == this->nil) {
        x = z->right;
        TransPlant(z, z->right);
    } else if (z->right == this->nil) {
        x = z->left;
        TransPlant(z, z->left);
    } else {
        y = Minimum(z->right);
        orig_color = y->color;
        x = y->right;
        if (p_of(y) == z) {
            p_of(x) = z;
        } else {
            TransPlant(y, y->right);
            y->right = z->right;
            p_of(y->right) = y;
        }
        TransPlant(z, y);
        y->left = z->left;
        p_of(y->left) = z;
        y->color = z->color;
    }
    if (orig_color == BLACK) {
        DeleteFixup(x);
    }
    printf("----------Delete %d----------\n", key);
    //Print();

    return y;
}

void RBTree::LeftRotate(rb_node_t *x)
{
    rb_node_t *y = x->right;
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

void RBTree::RightRotate(rb_node_t *y)
{
    rb_node_t *x = y->left;
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

static void print_tree(rb_node_t *root, rb_node_t *nil, int indent)
{
    rb_node_t *p = root;

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

rb_node_t *RBTree::Insert(rb_node_t *z)
{
     rb_node_t *x = this->root;
     rb_node_t *y = this->nil;

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
  while(p != this->nil) {
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
    if (n == this->nil)
        return NULL;

    while(n->left != this->nil)
        n = n->left;

    return n;
}

rb_node_t *RBTree::Next(rb_node_t *node)
{
    rb_node_t *p = NULL;
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
