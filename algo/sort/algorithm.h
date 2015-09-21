#ifndef SORT_ALGRITHM_H__
#define SORT_ALGRITHM_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ASSERT(x)							\
  if (!(x)) {								\
    printf("\033[31mFAILED: %s:%d %s\033[0m\n", __FILE__, __LINE__, #x); \
    exit(1);								\
  }

// utilies
static inline void print_array(int *array, int len)
{
  int i=0; 
  for (; i<len; ++i)
    printf("%-6d%s", array[i], ((i+1)%16 == 0)?"\n  \\   ":"");
  printf("\n");
}

static inline int is_sorted(int *a, int n)
{
    int i;
    for(i=1; i<n; ++i)
	if (a[i-1] > a[i]) return 0;
    return 1;
}


static inline void swap(int *x, int *y)
{
  int tmp = *x;
  *x = *y;
  *y = tmp;
}

extern int *array_create(int n, int desc);
extern int *array_dup(int *array, int n);
extern int array_comp(int *x, int *y, int n);
extern double timing_begin();
extern double timing_end(double start);


// sorting algorithms
// http://en.wikipedia.org/wiki/Sorting_algorithm
extern void quick_sort_3way(int *array, int lo, int hi);
extern void quick_sort(int *array, int lo, int hi);
extern void merge_sort(int *array, int p, int r);
extern void shell_sort(int *array, int len, int *delta, int t);
extern void insert_sort(int *array, int len);
extern void bubble_sort(int *array, int len);
extern void select_sort(int *array, int len);
extern void heap_sort(int *array, int len);
// search algorithm
int bin_search(int *array, int lo, int hi, int key);

#endif
