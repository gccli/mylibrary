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
  if (u->parent == NULL)
    this->root = v;
  else if (u->parent->left == u)
    u->parent->left = v;
  else if (u->parent->right == u)
    u->parent->right = v;
  if (v != NULL)
    v->parent = u->parent;
}

void BST::Delete(int key)
{
  BiNode_t *p = Search(key);
  BiNode_t *y, *q, *s;
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

void BST::left_rotate(BiNode_t *pivot)
{
  if (!pivot) return ;

  BiNode_t *y = pivot->right;
  pivot->right = y->right;
  y->parent = pivot->parent;
  if (pivot->parent == NULL)
    this->root =y;
  else if (pivot == pivot->parent->left)
    pivot->parent->left = y;
  else
    pivot->parent->right = y;
  y->left = pivot;
  pivot->parent = pivot;
}

void BST::right_rotate(BiNode_t *pivot)
{
  if (!pivot) return ;

  BiNode_t *y = pivot->right;
  pivot->right = y->right;
  y->parent = pivot->parent;
  if (pivot->parent == NULL)
    this->root =y;
  else if (pivot == pivot->parent->left)
    pivot->parent->left = y;
  else
    pivot->parent->right = y;
  y->left = pivot;
  pivot->parent = pivot;
}

