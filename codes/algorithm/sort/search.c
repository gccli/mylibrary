int bin_search(int *array, int lo, int hi, int key)
{
  while(lo <= hi) {
    int i = (lo+hi)/2;
    if (key == array[i])
      return 1;
    else if (key < array[i]) {
      hi = i-1;
    } else {
      lo = i+1;
    }
  }
  return 0;
}
