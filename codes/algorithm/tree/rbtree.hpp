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

    virtual BiNode_t *Minimum(BiNode_t *);
    virtual BiNode_t *Maximum(BiNode_t *);


    BiNode_t *First();
    BiNode_t *Next(BiNode_t *node);

    BiNode_t *Search(int key);
    BiNode_t *Insert(BiNode_t *);
    BiNode_t *Delete(BiNode_t *);
    void Print();

protected:
    virtual void TransPlant(BiNode_t *u, BiNode_t *v);
    void InsertFixup(BiNode_t *);
    void DeleteFixup(BiNode_t *);

    void RightRotate(BiNode_t *);
    void LeftRotate(BiNode_t *);

private:
    BiNode_t *nil;
};

#endif
