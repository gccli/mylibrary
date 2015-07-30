#include "sort/algorithm.h"
#include <stdio.h>
#include <stdlib.h>

static void shuffle(int *a, int n)
{
    int i,j;
    for(i=0; i<n; ++i) {
        j = random()%(n-2)+1;
        if (i % 2 == 0) {
            swap(&a[0], &a[j]);
        } else {
            swap(&a[n-1], &a[j]);
        }
    }
}

static int partition(int *a, int p, int r)
{
    int v = a[r];
    int i,j=p-1;

    for(i=p; i<r; ++i) {
        if (a[i] < v) {
            j++;
            swap(&a[i], &a[j]);
        }
    }
    swap(&a[j+1], &a[r]);
    return j+1;
}


int find_ith_number(int *a, int p, int r, int i)
{
    if (p == r) return a[p];

    int q = partition(a, p, r);
    int k = q-p+1;
    if (k == i) return a[q];
    else if (k > i)
        return find_ith_number(a, p, q-1, i);
    else
        return find_ith_number(a, q+1, r, i-k);
}

int main(int argc, char *argv[])
{
    int i=0;
    int n=20;
    int a[100];
    for(i=0; i<n; ++i) a[i] = i+1;
    shuffle(a, n);
    print_array(a,n);
    i=n/2;
    if (argc > 1) i = atoi(argv[1]);

    printf("find %d'th smallest number: %d\n",
           i, find_ith_number(a, 0, n-1, i));

    return 0;
}
