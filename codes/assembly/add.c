int accum = 0;
int sum(int x, int y)
{
  int s = x + y;
  accum += s;
  return s;
}
int simple(int *xp, int y)
{
  int t = *xp + y;
  *xp = t;
  return t;
}
