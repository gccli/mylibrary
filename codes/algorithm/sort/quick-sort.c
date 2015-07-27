#include "algorithm.h"
// http://en.wikipedia.org/wiki/Quicksort
#ifdef _test
int a[] = {13,19,9,5,8,7,4,21,2,6,11};
int n=sizeof(a)/sizeof(int);
#endif


static int partition_v1(int *a, int lo, int hi)
{
  int v = a[hi]; // pivot value
  int i = lo-1, j=lo;
  for (; j<hi; ++j) { // j = lo to hi-1
      if (a[j] <= v) {
	  i++;
	  swap(&a[i], &a[j]);
      }
  }
  swap(&a[i+1], &a[hi]);

  return i+1;
}

static int partition_v2(int *a, int lo, int hi)
{
  int v = a[lo]; // pivot value
  int i = lo, j = hi+1;
  while(1) {
    while(a[++i] < v) if (i==hi) break;
    while(v < a[--j]) if (j==lo) break;
    if (i>=j) break;
    swap(&a[i], &a[j]);

  }
  swap(&a[j], &a[lo]);

  return j;
}

#define partition partition_v1
void quick_sort(int *array, int lo, int hi)
{
  if (lo < hi) {
    // Pick an element, called a pivot, from the array
    /* Reorder the array so that all elements with values less than the pivot come before the pivot,
       while all elements with values greater than the pivot come after it (equal values can go either way).
       After this partitioning, the pivot is in its final position. */
    int p = partition(array, lo, hi);

#ifdef _test
    print_array(array, n);
#endif
    /* Recursively apply the above steps to the sub-array of elements with smaller values and 
       separately to the sub-array of elements with greater values.*/
    quick_sort(array, lo, p-1);
    quick_sort(array, p+1, hi);
  }
}

void quick_sort_3way(int *array, int lo, int hi)
{
  if (hi <= lo) return ;
  int lt=lo, gt=hi, i=lo;
  int pivot = array[lo];

  while(i <= gt) {
    if (array[i] < pivot) {
      swap(&array[lt], &array[i]);
      lt++;
      i++;
    } else if (array[i] > pivot) {
      swap(&array[i], &array[gt]);
      gt--;
    } else {
      i++;
    }
  }
  quick_sort_3way(array, lo, lt-1);
  quick_sort_3way(array, gt+1, hi);
}

#ifdef _test
// gcc -I. -g -D_test sort/quick-sort.c -o /tmp/s
int main(int argc, char *argv[])
{
    print_array(a, n);
    quick_sort(a, 0, n-1);
    print_array(a, n);
}
#endif
