#include "bstree.hpp"
#include <assert.h>

BiNode_t *BST::Insert(BiNode_t* node)
{
  BiNode_t *x = this->root;
  BiNode_t *p = NULL;
  while(x) {
    p = x;
    if (node->key < x->key)
      x = x->left;
    else
      x = x->right;
  }
  node->parent = p;
  if (!p) {
    this->root = node;
  } else if (p->key > node->key) {
    p->left = node;
  } else {
    p->right = node;
  }
  return node;
}

void BST::transplant(BiNode_t *u, BiNode_t *v)
{
    if (PARENT(u) == NULL)
        this->root = v;
    else if (PARENT(u)->left == u)
        PARENT(u)->left = v;
    else if (PARENT(u)->right == u)
        PARENT(u)->right = v;
    if (v != NULL)
        v->parent = u->parent;
}

void BST::Delete(int key)
{
  BiNode_t *p = Search(key);
  BiNode_t *q, *s;
  if (!p) return ;
  if (!LEFT(p)) {
    transplant(p, p->right);
    delete p;
  }
  else if(!RIGHT(p)) {
    transplant(p, p->left);
    delete p;
  }
  else {
    /**
     * Delete p: replace p with the minimum of p's right subtree or the maximum of p's left subtree
     * The following method find the maximum of p's left subtree
     */
    q = p;
    s = p->left;
    while(s->right) {q = s; s = s->right;}
    p->key = s->key;
    if (p != q)
      transplant(q->right, s->left);//q->right = s->left;
    else
      transplant(q->left, s->left); //q->left = s->left;
    assert(s != NULL && s->key != INVALID_KEY);
    delete s;
  }
}

BiNode_t *BST::Search(int key)
{
  BiNode_t *p = this->root;
  while(p) {
    if (key == p->key)
      return p;
    else if (key < p->key)
      p = p->left;
    else
      p = p->right;
  }

  return NULL;
}
