#include <math.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include "rbtree.h"

inline int visit(rb_node_t *p) {
    printf("%d ", p->key);
    return 0;
}
inline int visit_p(rb_node_t *p) {return 0;}
static int *array_create(int n, int desc)
{
  srandom((unsigned int )time(NULL));
  int *x = (int *)calloc(n, sizeof(int));
  for (int i=0; i<n; ++i) {
    x[i] = random()%10000;
    if (desc) x[i] = n-i;
  }

  return x;
}
double timing_begin()
{
  double start = 0.0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  start = 1.0*tv.tv_sec + 0.000001*tv.tv_usec;

  return start;
}
double timing_end(double start)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double end = 1.0*tv.tv_sec + 0.000001*tv.tv_usec;

  return end - start;
}

////////////////////////////////////////////////////////////////
//////// Test traversing binary tree

BTree *create_bstree(const int *array, int n)
{
  assert(array && n > 0);
  BTree *T = new BST;
  for (int i=0; i<n; ++i) {
    rb_node_t *node = new rb_node_t(array[i]);
    T->Insert(node);
  }
  return T;
}

void btree_visit(const int *a, int n)
{
    printf("\nSEQ: ");
    for(int i=0; i<n; ++i) printf("%d ", *(a+i));
    printf("\n");

    BTree *binary_tree = create_bstree(a, n);
    printf("PRE-ORDER\n");
    binary_tree->preorder_recursive(visit); printf("\n");
    binary_tree->preorder(visit); printf("\n\n");

    printf("IN-ORDER\n");
    binary_tree->inorder_recursive(visit); printf("\n");
    binary_tree->inorder(visit); printf("\n\n");

    printf("POST-ORDER\n");
    binary_tree->postorder_recursive(visit); printf("\n");
    binary_tree->postorder(visit); printf("\n");

    delete binary_tree;
}

void test_traverse_btree()
{
    int aa[][10] = {
      {7, 5, 10, 1, 6, 9, 11},
      {11, 5, 10, 1, 6, 9, 7},
      {1, 5, 10, 7, 6, 9, 11},
    };
    for(int i=0; i<3; ++i) btree_visit(aa[i], 7);
}

void test_delete(int n)
{
  int k = n;
  BTree *T = NULL;

  int a[] = {100, 50, 200, 40, 30, 35, 60, 44, 46, 45, 48, 47, 150, 300, 300, 300};
  n = sizeof(a)/sizeof(int);
  printf("\nSEQ: ");
  for(int i=0; i<n; ++i) printf("%d ", *(a+i));
  printf("\n");

  T = create_bstree(a, n);
  T->preorder(visit); printf("\n");
  T->inorder(visit); printf("\n\n");
  for(int i=0; i<n; ++i) {
    T->Delete(a[i]);
    T->preorder(visit); printf("\n");
    T->inorder(visit); printf("\n\n");
  }
  delete T;

  int *aa = array_create(k, 0);
  T = create_bstree(aa, k);
  T->inorder(visit_p);
  for(int i=0; i<k; ++i) {
    T->Delete(aa[i]);
  }

  T->inorder(visit);
  free(aa);

  delete T;
}

////////////////////////////////////////////////////////////////
//////// Test traversing binary tree

extern void print_complete_bt(const rb_node_t *root, int height);
extern void print_rb_tree(rb_node_t *root);
void test_rbtree(int length)
{
    rb_node_t *n, *p;
    int i,j,h,tmp;
    //int a[] = {39,3,7,12,10,14,15,16,17,19,20,21,23,26,41,30,47,28,38,35};
    //const int cnt = sizeof(a)/sizeof(int);
    int a[100];
    const int cnt = 7;
    for(i=0; i<cnt;++i)
        if (i%2 == 0)
            a[i] = 50-i;
        else
            a[i] = 50+i;

    RBTree T;

    for(i=0; i<cnt; ++i) {
        n = new rb_node_t(a[i]);
        T.Insert(n);
    }
    h = rb_height(T.Root());
    printf("Number of node %d, height %d\n", cnt, h);
    print_rb_tree(T.Root());

    print_complete_bt(T.Root(), h);

    for(i=0; i<cnt; ++i) {
        j = random()%(cnt-1);
        tmp = a[j];
        a[j] = a[i%2];
        a[i%2] = tmp;
    }

    for(i=0; i<cnt; ++i) {
        p = T.Search(a[i]);
        T.Delete(p);
    }
}

int main(int argc, char* argv[])
{
  int n = 1000;
  if (argc == 2) n = atoi(argv[1]);
  //test_delete(n);
  test_rbtree(n);

  return 0;
}
