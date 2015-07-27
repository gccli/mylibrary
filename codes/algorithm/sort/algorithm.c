#include <sys/time.h>
#include "algorithm.h"

int *array_create(int n, int desc)
{
  srandom((unsigned int )time(NULL));

  int *x = calloc(n, sizeof(int));
  int i;
  for (i=0; i<n; ++i) {
    x[i] = random()%10000;
    if (desc) x[i] = n-i;
  }

  return x;
}

int *array_dup(int *array, int n)
{
  int *x = calloc(n, sizeof(int));
  int i;
  for (i=0; i<n; ++i)
    x[i] = array[i];
  return x;
}

int array_comp(int *x, int *y, int n)
{
  int i, ret = 0;
  for (i=0; i<n; ++i)
    if (x[i] < y[i]) {
      ret = -1;
      break;
    } else if (x[i] > y[i]) {
      ret = 1;
      break;
    }
  return ret;
}


double timing_begin()
{
  double start = 0.0;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  start = 1.0*tv.tv_sec + 0.000001*tv.tv_usec;

  return start;
}

double timing_end(double start)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  double end = 1.0*tv.tv_sec + 0.000001*tv.tv_usec;

  return end - start;
}

