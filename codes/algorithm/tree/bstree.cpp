#include "bstree.h"
#include <assert.h>

rb_node_t *BST::Insert(rb_node_t* node)
{
  rb_node_t *x = this->root;
  rb_node_t *p = NULL;
  while(x) {
    p = x;
    if (node->key < x->key)
      x = x->left;
    else
      x = x->right;
  }
  node->p = p;
  if (!p) {
    this->root = node;
  } else if (p->key > node->key) {
    p->left = node;
  } else {
    p->right = node;
  }
  return node;
}

void BST::TransPlant(rb_node_t *u, rb_node_t *v)
{
    if (p_of(u) == NULL)
        this->root = v;
    else if (p_of(u)->left == u)
        p_of(u)->left = v;
    else if (p_of(u)->right == u)
        p_of(u)->right = v;
    if (v != NULL)
        v->p = u->p;
}


rb_node_t *BST::Minimum(rb_node_t *z)
{
    while (z->left)
        z = z->left;
    return z;
}

rb_node_t *BST::Maximum(rb_node_t *z)
{
    while (z->right)
        z = z->right;
    return z;
}

rb_node_t *BST::Successor(rb_node_t *z)
{
    rb_node_t *p;
    if (z->right) {
        return Minimum(z->right);
    }

    p = p_of(z);
    while(p && z == p->right) {
        z = p;
        p = p_of(p);
    }
    return p;
}

rb_node_t *BST::Predecessor(rb_node_t *z)
{
    rb_node_t *p;
    if (z->left) {
        return Maximum(z->left);
    }
    p = p_of(z);
    while (p && z == p->left) {
        z = p;
        p = p_of(p);
    }
    return p;
}

void BST::Delete(int key)
{
  rb_node_t *p = Search(key);
  rb_node_t *q, *s;
  if (!p) return ;
  if (!p->left) {
    TransPlant(p, p->right);
    delete p;
  }
  else if(!p->right) {
    TransPlant(p, p->left);
    delete p;
  }
  else {
    /**
     * Delete p: replace p with the minimum of p's right subtree
     *   or the maximum of p's left subtree
     * The following method find the maximum of p's left subtree
     */
    q = p;
    s = p->left;
    while(s->right) {q = s; s = s->right;}
    p->key = s->key;
    if (p != q)
      TransPlant(q->right, s->left);//q->right = s->left;
    else
      TransPlant(q->left, s->left); //q->left = s->left;

    delete s;
  }
}

rb_node_t *BST::Search(int key)
{
  rb_node_t *p = this->root;
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
