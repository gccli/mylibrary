#include <stdint.h>
#include "algorithm.h"

// p, q, and r are indices of the array
// such that p <= q < r
// n = r-p+1 total number of elements being merge

static void merge(int *array, int p, int q, int r)
{
  int n1 = q-p+1;
  int n2 = r-q;

  int *L = calloc(sizeof(int), n1+1);
  int *R = calloc(sizeof(int), n2+1);

  int i,j,k;
  for(i=0; i<n1; ++i)
    L[i] = array[i+p];
  for(j=0; j<n2; ++j)
    R[j] = array[j+q+1];

  L[n1] = R[n2] = 0x7fffffff; // sentinel

  i=j=0;
  for (k=p; k<=r; ++k)  // p and r is indices of array, subarray length is r-p+1
  {
    if (L[i] <= R[j])
      array[k] = L[i++];
    else
      array[k] = R[j++];
  }

  free(L);
  free(R);
}

// no sentinel
void merge_without_sentinel(int *array, int p, int q, int r)
{
  int n1 = q-p+1;
  int n2 = r-q;

  int *L = calloc(sizeof(int), n1+1);
  int *R = calloc(sizeof(int), n2+1);

  int i,j,k;
  for(i=0; i<n1; ++i)
    L[i] = array[i+p];
  for(j=0; j<n2; ++j)
    R[j] = array[j+q+1];

  i=j=0;
  for (k=p; k<=r; ++k)  // p and r is indices of array, subarray length is r-p+1
  {
    if (i == n1) {
      array[k] = R[j++];
      continue;
    } 
    if (j == n2) {
      array[k] = L[i++];
      continue;
    }
    if (L[i] <= R[j])
      array[k] = L[i++];
    else
      array[k] = R[j++];
  }

  free(L);
  free(R);
}


// p and r indices of array
// n=r-p+1
// the subarray has at most one element and is therefore already sorted
// divide array into array[p...q] and array[q+1...r], each containing n/2 and n/2+1 elements
void merge_sort(int *array, int p, int r)
{
  if (p < r)
  {
    int q = (p+r)/2;
    merge_sort(array, p, q);
    merge_sort(array, q+1, r);
    merge_without_sentinel(array, p, q, r);
  }
}
