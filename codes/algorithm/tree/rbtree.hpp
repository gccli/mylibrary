#ifndef RB_TREE_H__
#define RB_TREE_H__

#include "bstree.hpp"

class RBTree : public BST {
public:
  RBTree() {
    nil = new BiNode_t(INVALID_KEY);
    nil->color = BLACK;
  }

  virtual BiNode_t *Insert(BiNode_t *);

private:
  BiNode_t *nil;
};

#endif
