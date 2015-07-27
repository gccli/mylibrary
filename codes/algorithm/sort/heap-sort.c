#include "heap.h"

void heap_sort(int *array, int len)
{
  int i;

  build_max_heap(array, len);
  for (i = len - 1; i>0; --i) {
      max_heapify(array, 0, i);
      swap(&array[0], &array[i]);
  }
}


#ifdef _test
int main(int argc, char *argv[])
{
    int i;
    int a[] = {1,2,9,88,5,-3,0,12,-7,44,23,8,-9,4,5};
    int n = sizeof(a)/sizeof(int);
    heap_t max_heap, min_heap;

    max_heap_init(&max_heap, a, n);
    min_heap_init(&min_heap, a, n);
    print_array(a, n);
    heap_sort(a, n);
    print_array(a, n);
    printf("Sorted %s\n", is_sorted(a, n)?"Yes":"No");

    while ((n = max_heap_pop(&max_heap)) != UNDER_FLOW)
	printf("%d ", n);
    puts("");
    while ((n = min_heap_pop(&min_heap)) != UNDER_FLOW)
	printf("%d ", n);
    puts("");

    return 0;
}
#endif
