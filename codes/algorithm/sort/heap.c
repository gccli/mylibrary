#include "heap.h"

// Min Heap
void min_heapify(int *a, int i, int hi)
{
    int l = left(i);
    int r = right(i);
    int smallest = i;

    if (l <= hi && a[l] < a[i])
	smallest = l;

    if (r <= hi && a[r] < a[smallest])
	smallest = r;

    if (smallest != i) {
	swap(&a[i], &a[smallest]);
	min_heapify(a, smallest, hi);
    }
}

void build_min_heap(int *a, int n)
{
  int i;
  for(i=n/2; i>=0; --i)
      min_heapify(a, i, n-1);
}

int min_heap_pop(heap_t *a) 
{
    int min;
    if (a->last == 0) {
	return UNDER_FLOW;
    }
    min = a->data[0];
    a->data[0] = a->data[a->last];
    a->last -= 1;
    min_heapify(a->data, 0, a->last);

    return min;
}

void min_heap_decrease(heap_t *a, int i, int key)
{
    if (key > a->data[i]) {
	printf("%s error\n", __FUNCTION__);
	return ;
    }
    a->data[i] = key;
    while(i > 0 && a->data[i] < a->data[parent(i)]) {
	swap(&a->data[i], &a->data[parent(i)]);
	i = parent(i);
    }
}

void min_heap_push(heap_t *a, int key) 
{
    a->last += 1;
    a->data[a->last] = INT32_MAX;
    min_heap_decrease(a, a->last, key);
}

void min_heap_init(heap_t *heap, int *a, int n)
{
    int i;
    heap->data = calloc(sizeof(int), n);
    heap->last = 0;
    heap->size = n;
    for(i=0; i<n; ++i) {
	heap->data[i] = INT32_MAX;
    }

    for(i=0; i<n; ++i) {
	min_heap_push(heap, a[i]);
    }
}

// Max Heap
void max_heapify(int *a, int i, int hi)
{
    int l = left(i);
    int r = right(i);
    int largest = i;
    
    if (l <= hi && a[l] > a[i])
	largest = l;

    if (r <= hi && a[r] > a[largest])
	largest = r;

    if (largest != i) {
	swap(&a[i], &a[largest]);
	max_heapify(a, largest, hi);
    }
}

void build_max_heap(int *a, int n)
{
  int i;
  for(i=n/2; i>=0; --i)
      max_heapify(a, i, n-1);
}

int max_heap_pop(heap_t *a) 
{
    int max;
    if (a->last == 0) {
	return UNDER_FLOW;
    }
    max = a->data[0];
    a->data[0] = a->data[a->last];
    a->last -= 1;
    max_heapify(a->data, 0, a->last);

    return max;
}

int max_heap_increase(heap_t *a, int i, int key)
{
    if (key < a->data[i]) {
	printf("new key smaller than current key\n");
	return ;
    }
    a->data[i] = key;
    while (i > 0 && a->data[parent(i)] < a->data[i]) {
	swap(&a->data[i], &a->data[parent(i)]);
	i = parent(i);
    }

    return i;
}

void max_heap_push(heap_t *a, int key)
{
    a->last += 1;
    a->data[a->last] = INT32_MIN;
    max_heap_increase(a, a->last, key);
}

void max_heap_init(heap_t *heap, int *a, int n)
{
    int i;
    heap->data = malloc(sizeof(int)*n);
    heap->size = n;
    heap->last = 0;
    for(i=0; i<n; ++i) {
	heap->data[i] = INT32_MIN;
    }
    for(i=0; i<n; ++i) {
	max_heap_push(heap, a[i]);
    }
}
