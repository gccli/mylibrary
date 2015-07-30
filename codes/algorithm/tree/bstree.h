#ifndef BINARY_SEARCH_TREE_H__
#define BINARY_SEARCH_TREE_H__

#include "btree.h"

class BST : public BTree
{
public:
    BST(){}
    virtual ~BST(){}

    virtual void Delete(int key);
    virtual rb_node_t *Insert(rb_node_t *);
    virtual rb_node_t *Search(int key);

    virtual rb_node_t *Minimum(rb_node_t *);
    virtual rb_node_t *Maximum(rb_node_t *);
    virtual rb_node_t *Successor(rb_node_t *);
    virtual rb_node_t *Predecessor(rb_node_t *);

protected:
    virtual void TransPlant(rb_node_t *u, rb_node_t *v);
};
#endif
