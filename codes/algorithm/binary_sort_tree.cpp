#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct BiTNode
{
  int             key;
  void           *value;
  struct BiTNode *lchild;
  struct BiTNode *rchild;
  BiTNode() {
    value = NULL;
    lchild = rchild = NULL;
  }
} BiTNode, *BiTree;

bool BiTreePrint(BiTNode *node)
{
  if (node == NULL) assert(0);
  printf("%d->", node->key);

  return true;
}

bool BiTreeTraverse(BiTree T, bool (*op)(BiTNode *))
{
  if (T != NULL) {
    if (T->lchild) {
      BiTreeTraverse(T->lchild, op);  
    }
    if(op(T))
      BiTreeTraverse(T->rchild, op);
  }
  return true;
}

bool BiTreePreTraverse(BiTree T, bool (*op)(BiTNode *))
{
  if (T != NULL) {
    if (op(T)) {
      if (BiTreePreTraverse(T->lchild, op)) {
	if (BiTreePreTraverse(T->rchild, op))
	  return true;
      }
    }
  }
  return true;
}

bool BiTreeSearch(BiTree T, int key, BiTree f, BiTree *p)
{
  if (T == NULL) {
    *p = f;
    return false;
  }
  else if (key == T->key) {
    *p = T;
    return true;
  }
  else if (key < T->key) {
    return BiTreeSearch(T->lchild, key, T, p);
  }
  return BiTreeSearch(T->rchild, key, T, p);
}

bool BiTreeInsert(BiTree *T, int key)
{
  BiTree s = NULL;
  if (!BiTreeSearch(*T, key, NULL, &s)) {
    BiTNode *node = new BiTNode;
    node->key = key; 

    if (s == NULL) 
      *T = node;
    else if (key < s->key)
      s->lchild = node;
    else
      s->rchild = node;
    return true;
  }
  
  return false;
}

bool BiTreeDeleteInternal(BiTree *T)
{
  BiTree p = *T;
  BiTree q = NULL;
  BiTree s = NULL;
  if (p->rchild == NULL) {
    q = p; p = p->lchild;
    delete q;
  }
  else if (p->lchild == NULL) {
    q = p; p = p->rchild;
    delete q;
  }
  else {
    q = p; s = p->lchild;
    while(s->rchild) { q = s; s = s->rchild; }
    p->key = s->key;
    if (q->key != p->key)
      q->rchild = s->lchild;
    else 
      q->lchild = s->lchild;
    delete s;
  }
  *T = p;
  return true;
}

bool BiTreeDelete(BiTree *T, int key)
{
  if (T == NULL || *T == NULL) return false;

  if ((*T)->key == key) 
    return BiTreeDeleteInternal(T);
  else if (key < (*T)->key)
    return BiTreeDelete(&((*T)->lchild), key);
  return BiTreeDelete(&((*T)->rchild), key);
}

int main(int argc, char *argv[])
{
  int key[] = {23,38,21,61,58,3,90,12,9,65,51,20,30,24,17,1,19,91,6,56};
  int i, length = sizeof(key)/sizeof(int);

  BiTree T = NULL;
  printf("Original Sequence, lengthis %d\n", length);
  for (i=0; i<length; ++i) {
    printf("%02d ", key[i]);
  }

  printf("\n");
  printf("\nCreate Tree ");
  for (i=0; i<length; ++i) {
    BiTreeInsert(&T, key[i]);
    printf("\n");BiTreePreTraverse(T, BiTreePrint);  
  }

  printf("\n");
  printf("\nDelete Tree");
  for (i=0; i<length; ++i) {
    printf("\n");BiTreeTraverse(T, BiTreePrint);
    BiTreeDelete(&T, key[i]);
  }
  printf("\n");

  return 0;
}
