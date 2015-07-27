#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

// http://en.wikipedia.org/wiki/ANSI_escape_code


#define CLS            "\033[1;1H\033[0J"
#define TCKL           "\033[0G\033[0K"
#define CursorUp       "\033[A"
#define CursorDown     "\033[B"
#define CursorForward  "\033[C"
#define CursorBack     "\033[D"
#define CursorNL       "\033[E"
#define CursorPL       "\033[F"
#define ScrollUp       "\033[S"
#define ScrollDown     "\033[T"

// Process has done i out of n rounds,
// and we want a bar of width w and resolution r
static inline void progressbar(int x, int n, int r, int w)
{
  // Only update r times.
  if (n < r) r=n;
  if (x%(n/r)!=0 || x==0) return ;
 
  // Calculuate the ratio of complete-to-incomplete.
  float ratio = x/(float)n;
  int   c     = ratio * w;
 
  printf("+OK %3d/%-3d [", x, n);
  for (x=0; x<c; x++)
    printf("=");
  printf(">");
  for (x=c+1; x<w; x++)
    printf(" ");
  printf("] %.2f%% ", ratio*100);
  fflush(stdout);  printf(TCKL"");
}

char star[] = {'/','-','\\','|'};
int loop(int n)
{
  int i;
  for (i=0; i<n; ++i)
  {
    //fprintf(stderr, TCKL"%2c   %d/%d", star[i%4], i, n);
    progressbar(i, n, 100, 80);
    usleep(5000);
  }

  return 0;
}

int main(int argc, char *argv[])
{
  int i,total=0;

  for (i=0; i<1000; ++i)
  {
    int n = 500+rand()%500;
  
    total += n;
    fprintf(stderr, CLS"\n");
    fprintf(stderr, TCKL"Total loop %d  +%d", total, n);
    fprintf(stderr, "\n"ScrollUp""CursorPL);
    loop(n);
  }

  return 0;
}
