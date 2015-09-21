#include "algorithm.h"
#include <math.h>

void insert_sort(int *array, int len)
{
  int i,j,tmp = 0;
  for(i=1; i<len; ++i) {
    // insert array[i] into the sorted sequence array[0 ~ i-1]
    tmp = array[i];
    j=i;
    while(array[j-1] > tmp) {
      array[j] = array[j-1];
      if (--j == 0) break;
    }
    array[j] = tmp;
  }
}


void shell_insert(int *a, int n, int k)
{
    int i,j,tmp;
    for(i=k; i<n; i+=k) {
	tmp = a[i];
	for(j=i-k; j>=0 && a[j] > tmp; j-=k) 
	    a[j+k] = a[j];
	a[j+k] = tmp;
    }
}

void shell_sort(int *array, int len, int *delta, int t)
{
    int k;
    for(k=0; k<t; ++k) {
	shell_insert(array, len, delta[k]);
    }
}

#ifdef _test
int main(int argc, char *argv[])
{
  int a[] = {1,2,9,7,4,5,8,0,3,6};
  print_array(a, 10);
  //shell_sort(a, 10);
  printf("Sorted %s\n", is_sorted(a, 10)?"Yes":"No");

  return 0;
}
#endif
