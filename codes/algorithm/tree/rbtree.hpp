#ifndef RB_TREE_H__
#define RB_TREE_H__

#include "bstree.hpp"

class RBTree : public BST {
public:
    RBTree() {
        nil = new BiNode_t;
        nil->p = nil;
        this->root = nil;
    }

    BiNode_t *First();
    BiNode_t *Next(BiNode_t *node);

    BiNode_t *Insert(BiNode_t *);
    void InsertFixup(BiNode_t *);

    void RightRotate(BiNode_t *);
    void LeftRotate(BiNode_t *);

private:
    BiNode_t *nil;
};

#endif
