#ifndef BINARY_TREE_H__
#define BINARY_TREE_H__

#include <stdio.h>
#include <stdlib.h>

typedef enum {
    RED = 0,
    BLACK = 1
} color_t;

typedef struct _BTNode {
    _BTNode()
        :parent(0),left(0),right(0),color(BLACK),key(0)
    {}
    _BTNode(int k)
        :parent(0),left(0),right(0),color(BLACK),key(k)
    { }
    ~_BTNode() {
    }

    struct _BTNode *parent;
    struct _BTNode *left;
    struct _BTNode *right;
    color_t color;
    int key;
} BiNode_t;


#define parent_of(n) (n)->parent

typedef int (*VisitFunc_t)(BiNode_t *);

class BTree {
public:
  BTree() { root = NULL; }
  virtual ~BTree() {}

  virtual void Destroy();
  virtual void Delete(int key){}
  virtual BiNode_t *Search(int key){ return NULL; }
  virtual BiNode_t *Insert(BiNode_t *);

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
  void preorder_recursive_impl(BiNode_t *, VisitFunc_t);
  void inorder_recursive_impl(BiNode_t *, VisitFunc_t);
  void postorder_recursive_impl(BiNode_t *, VisitFunc_t);

protected:
    BiNode_t *root;
};


#endif
