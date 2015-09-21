#include "algorithm.h"

void select_sort(int *array, int len)
{
  int i,j,mini = 0;
  for(i=0; i<len; ++i) {
    mini = i;
    for(j=i+1; j<len; j++)
      if (array[mini] > array[j])
	mini = j;
    int tmp = array[i];
    array[i] = array[mini];
    array[mini] = tmp;
  }
}
