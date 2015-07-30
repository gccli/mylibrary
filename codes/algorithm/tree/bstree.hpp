#ifndef BINARY_SEARCH_TREE_H__
#define BINARY_SEARCH_TREE_H__

#include "btree.hpp"

class BST : public BTree {
public:
    BST(){}
    virtual ~BST(){}

    virtual BiNode_t *Insert(BiNode_t *);
    virtual void Delete(int key);
    virtual BiNode_t *Search(int key);

    virtual BiNode_t *Minimum(BiNode_t *);
    virtual BiNode_t *Maximum(BiNode_t *);
    virtual BiNode_t *Successor(BiNode_t *);
    virtual BiNode_t *Predecessor(BiNode_t *);

protected:
    virtual void TransPlant(BiNode_t *u, BiNode_t *v);
};



#endif
