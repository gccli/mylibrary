#ifndef BINARY_TREE_H__
#define BINARY_TREE_H__

#include <stdio.h>
#include <stdlib.h>

#define INVALID_KEY 0x7fffffff
#define RED 1
#define BLACK 2

typedef struct _BTNode {
  _BTNode(int k)
  :parent(0),left(0),right(0),key(k),color(0)
  { }
  ~_BTNode() {
    left=NULL;
    right=NULL;
    parent=NULL;
    color=0;    
    key = INVALID_KEY;
  }

  int key;
  struct _BTNode *parent;
  struct _BTNode *left;
  struct _BTNode *right;
  unsigned int color;

private:
  _BTNode(){}
} BiNode_t;

#define LEFT(n) n->left
#define RIGHT(n) n->right
#define PARENT(n) n->parent
#define ISLEFT(n) (PARENT(n) && LEFT(PARENT(n)) == n)
#define ISRIGHT(n) (PARENT(n) && RIGHT(PARENT(n)) == n)

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
