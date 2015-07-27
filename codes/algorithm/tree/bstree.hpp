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

  virtual void left_rotate(BiNode_t *);
  virtual void right_rotate(BiNode_t *);

protected:
  void transplant(BiNode_t *u, BiNode_t *v);
};



#endif
