#ifndef RB_TREE_H__
#define RB_TREE_H__

#include "bstree.h"

class RBTree : public BST
{
public:
    RBTree() {
        this->root = &sentry;
    }

    virtual rb_node_t *Minimum(rb_node_t *);
    virtual rb_node_t *Maximum(rb_node_t *);

    rb_node_t *First();
    rb_node_t *Next(rb_node_t *node);

    rb_node_t *Search(int key);
    rb_node_t *Insert(rb_node_t *);
    rb_node_t *Delete(rb_node_t *);

protected:
    virtual void TransPlant(rb_node_t *u, rb_node_t *v);
    void InsertFixup(rb_node_t *);
    void DeleteFixup(rb_node_t *);

    void RightRotate(rb_node_t *);
    void LeftRotate(rb_node_t *);
};




#endif
