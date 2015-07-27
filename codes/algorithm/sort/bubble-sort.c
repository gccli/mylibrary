#include "algorithm.h"

/*
void bubble_sort(int *array, int length)
{
  int i,j;
  for (i=0; i<length; ++i) {
    for (j=i+1; j<length; ++j) {
      if (array[i] > array[j])
	swap (&array[i], &array[j]);
    }
  }
}
*/

void bubble_sort(int *a, int n)
{
  int i,j;
  for (i=n; i>=0; --i) {
    for (j=1; j<n; ++j) {
      if (a[j-1] > a[j])
	swap (&a[j-1], &a[j]);
    }
  }
}
