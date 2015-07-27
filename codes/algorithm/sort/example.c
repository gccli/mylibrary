#include "algorithm.h"
#include <math.h>

int *delta;
int  delta_len;

typedef int (*compar)(const void *, const void *);
static inline int icomp(int *x, int *y) 
{
  return (*x > *y)?1:(*x < *y?-1:0);
}

static void quick3waysort(int *a, int n) {
  quick_sort_3way(a, 0, n-1);
}
static void quicksort(int *a, int n) {
  quick_sort(a, 0, n-1);
}

static void mergesort(int *a, int n) {
  merge_sort(a, 0, n-1);
}

static void shellsort(int *a, int n) {
    shell_sort(a, n, delta, delta_len);
}

int test(int *a, int n, int *sorted, const char *name, void (*sort)(int *a, int n)) 
{
  int *aa = array_dup(a,n);
  double start = timing_begin();
  sort(aa, n);
  printf("%s sort timecost : %lf\n", name, timing_end(start));
  if(array_comp(aa, sorted, n)) print_array(aa, n);  
  free(aa);
}

int main(int argc, char *argv[])
{
  int debug = 0, seq = 0, key = -1, n;
  double start, end;

  if (argc < 2) {
    printf("usage: %s <length> [cmd [args]]\n", argv[0]);
    printf("   debug\n");
    printf("   search key\n");
    return 0;
  }
  n = atoi(argv[1]);
  int k,t = (int) log2(n-1);
  delta = calloc(t+2, sizeof(int));
  for(k=0; k<=t; ++k) 
      delta[k] = 1+(int)pow(2, t-k);
  delta[k] = 1;
  delta_len = k+1;

  if (argc >= 3) {
    if (strcmp(argv[2], "debug") == 0) debug = 1;
    else if (strcmp(argv[2], "search") == 0) {
      if (argc == 4) key = atoi(argv[3]);
      debug = 1; seq = 1;
    }
  }
  int *a = array_create(n, seq);
  int *sorted = array_dup(a, n);

  start = timing_begin();
  qsort(sorted, n, sizeof(int), (compar)icomp);
  end = timing_end(start);
  if(debug) {
    printf("Original Array:\n");
    print_array(a, n);
    printf("Sorted Array:\n");
    print_array(sorted, n);
    print_array(delta, delta_len);
    printf("--------------------------------\n");
  }
  if(key >= 0) printf("Key %d %s\n", key, bin_search(sorted, 0, n-1, key)?"Found":"Not Found");
  printf("\n");

  printf("GNU qsort timecost : %lf\n", end);
  test(a,n,sorted,"Heap", heap_sort);
  test(a,n,sorted,"Quick 3-Way", quick3waysort);
  test(a,n,sorted,"Quick", quicksort);
  test(a,n,sorted,"Merge", mergesort);
  test(a,n,sorted,"Shell", shellsort);
  test(a,n,sorted,"Insert", insert_sort);
  test(a,n,sorted,"Select", select_sort);
  test(a,n,sorted,"Bubble", bubble_sort);

  free(a);
  free(sorted);
  if(delta) free(delta);

  return 0;
}
