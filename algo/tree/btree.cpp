#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <stack>
#include "btree.h"

static bool debug_on = false;
static inline void debug(const char *flag, int key) {
    if (!debug_on) return ;
    char buff[16] = {0};
    if (key >= 0) sprintf(buff, "%d", key);
    else sprintf(buff, "NULL");
    printf("<%s %s>", flag, buff);
}

////////////////////////////////////////////////////////////////
//////// B-Tree Implementation

void BTree::Destroy()
{
}

rb_node_t *BTree::Insert(rb_node_t *node)
{
  printf("Not yet implement!\n");
  abort();
  return NULL;
}

void BTree::preorder_recursive_impl(rb_node_t *root, VisitFunc_t visit)
{
  if (root == NULL) return ;
  visit(root);
  preorder_recursive_impl(root->left, visit);
  preorder_recursive_impl(root->right, visit);
}

void BTree::inorder_recursive_impl(rb_node_t *root, VisitFunc_t visit)
{
  if (root == NULL) return ;
  inorder_recursive_impl(root->left, visit);
  visit(root);
  inorder_recursive_impl(root->right, visit);
}

void BTree::postorder_recursive_impl(rb_node_t *root, VisitFunc_t visit)
{
  if (root == NULL) return ;
  postorder_recursive_impl(root->left, visit);
  postorder_recursive_impl(root->right, visit);
  visit(root);
}

void BTree::preorder(VisitFunc_t visit)
{
  if (root == NULL) return ;

  debug_on = false;
  std::stack<rb_node_t *> s;
  rb_node_t *top;
  s.push(this->root);
  while(!s.empty()) {
    top = s.top(); s.pop(); debug("POP", top->key);
    visit(top);
    if (top->right) { s.push(top->right); debug("PUSH", top->right->key); }
    if (top->left) { s.push(top->left); debug("PUSH", top->left->key); }
  } 
}

void BTree::inorder(VisitFunc_t visit)
{
  if (root == NULL) return ;

  debug_on = false;
  std::stack<rb_node_t *> s;
  rb_node_t *p = this->root;
  while(p != NULL || !s.empty()) {
    while(p != NULL) {
      debug("PUSH", p->key);
      s.push(p); p = p->left;
    }
    if (!s.empty()) {
      p = s.top(); s.pop();
      debug("POP", p->key);
      visit(p);
      p = p->right;
    }
  }
}

void BTree::postorder(VisitFunc_t visit)
{
  if (root == NULL) return ;

  debug_on = false;
  std::stack<rb_node_t *> s;
  rb_node_t *p = this->root;
  while(p || !s.empty()) {
    if (!p) {
      while(!s.empty() && p == s.top()->right) {
	p = s.top(); s.pop(); 
	visit(p);
	debug("POP", p->key);
      }
      p = s.empty() ? NULL : s.top()->right;
    }
    else {
      s.push(p);
      debug("PUSH", p->key);
      p = p->left;
    }
  }
}
