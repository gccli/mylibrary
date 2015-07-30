#ifndef BINARY_TREE_H__
#define BINARY_TREE_H__

#include <stdio.h>
#include <stdlib.h>

typedef enum
{
    RED = 0,
    BLACK = 1
} rb_color_t;

typedef struct _rb_node
{
    _rb_node()
        :p(0),left(0),right(0),color(BLACK),key(0)
    {}
    _rb_node(int k)
        :p(0),left(0),right(0),color(BLACK),key(k)
    {}
    ~_rb_node() {
    }

    struct _rb_node *p;           // parent
    struct _rb_node *left;
    struct _rb_node *right;
    rb_color_t       color;
    int key;
} rb_node_t;

#define p_of(n)  (n)->p
#define pp_of(n) p_of((n))->p

typedef int (*VisitFunc_t)(rb_node_t *);

class BTree
{
public:
    BTree() { root = NULL; }
    virtual ~BTree() {}

    virtual void Destroy();
    virtual void Delete(int key){}
    virtual rb_node_t *Search(int key){ return NULL; }
    virtual rb_node_t *Insert(rb_node_t *);

    void preorder_recursive(VisitFunc_t visit) {
        preorder_recursive_impl(this->root, visit);
    }
    void inorder_recursive(VisitFunc_t visit) {
        inorder_recursive_impl(this->root, visit);
    }
    void postorder_recursive(VisitFunc_t visit) {
        postorder_recursive_impl(this->root, visit);
    }
    void preorder(VisitFunc_t func);
    void inorder(VisitFunc_t func);
    void postorder(VisitFunc_t func);

protected:
    void preorder_recursive_impl(rb_node_t *, VisitFunc_t);
    void inorder_recursive_impl(rb_node_t *, VisitFunc_t);
    void postorder_recursive_impl(rb_node_t *, VisitFunc_t);

protected:
    rb_node_t *root;
};
#endif
