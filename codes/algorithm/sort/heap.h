#ifndef _heap_h__
#define _heap_h__

#include <stdio.h>
#include <stdint.h>
#include "algorithm.h"

typedef struct _heap_struct {
    size_t last; // the max index of heap
    size_t size; // the capacity of heap's elements
    int *data;
} heap_t;

#define left(i)        2*((i)+1)-1
#define right(i)       left(i)+1
#define parent(i)      ((i)-1)/2

#define UNDER_FLOW   INT32_MIN
#define OVER_FLOW    INT32_MAX

void max_heapify(int *a, int i, int hi);
void min_heapify(int *a, int i, int hi);

void build_max_heap(int *a, int n);
void build_min_heap(int *a, int n);

#endif
