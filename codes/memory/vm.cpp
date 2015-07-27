#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include <sys/resource.h>

/*
int getrusage(int who, struct rusage *usage);
getrusage()  returns  current resource usages, for a who of either RUSAGE_SELF or RUSAGE_CHILDREN.  The former asks for resources used by the current pro-
  cess, the latter for resources used by those of its children that have terminated and have been waited for.
*/
//  struct rusage {
//    struct timeval ru_utime; /* user time used */
//    struct timeval ru_stime; /* system time used */
//    long   ru_maxrss;        /* maximum resident set size */
//    long   ru_ixrss;         /* integral shared memory size */
//    long   ru_idrss;         /* integral unshared data size */
//    long   ru_isrss;         /* integral unshared stack size */
//    long   ru_minflt;        /* page reclaims */
//    long   ru_majflt;        /* page faults */
//    long   ru_nswap;         /* swaps */
//    long   ru_inblock;       /* block input operations */
//    long   ru_oublock;       /* block output operations */
//    long   ru_msgsnd;        /* messages sent */
//    long   ru_msgrcv;        /* messages received */
//    long   ru_nsignals;      /* signals received */
//    long   ru_nvcsw;         /* voluntary context switches */
//    long   ru_nivcsw;        /* involuntary context switches */
//  };

void show_rusage(struct rusage* ru)
{
    printf ("      ru_utime:    %d.%06d\n", ru->ru_utime.tv_sec, ru->ru_utime.tv_usec);
    printf ("      ru_stime:    %d.%06d\n", ru->ru_stime.tv_sec, ru->ru_stime.tv_usec);
    printf ("      ru_maxrss:   %d (KB), maximum resident set size\n", ru->ru_maxrss);
    printf ("      ru_idrss:    %d (KB), integral shared memory size\n", ru->ru_ixrss);
    printf ("      ru_idrss:    %d (KB), integral unshared data size\n", ru->ru_idrss);
    printf ("      ru_isrss:    %d (KB), integral unshared stack size\n", ru->ru_isrss);
    printf ("      ru_minflt:   %d, page reclaims\n", ru->ru_minflt);
    printf ("      ru_majflt:   %d, page faults\n", ru->ru_majflt);
    printf ("      ru_nswap:    %d, swaps\n", ru->ru_nswap);
    printf ("      ru_inblock:  %d, block input operations\n", ru->ru_inblock);
    printf ("      ru_oublock:  %d, block output operations\n", ru->ru_oublock);
    printf ("      ru_msgsnd:   %d, messages sent\n", ru->ru_msgsnd);
    printf ("      ru_msgrcv:   %d, messages received\n", ru->ru_msgrcv);
    printf ("      ru_nsignals: %d, signals received\n", ru->ru_nsignals);
    printf ("      ru_nvcsw:    %d, voluntary context switches\n", ru->ru_nvcsw);
    printf ("      ru_nivcsw:   %d, involuntary context switches\n", ru->ru_nivcsw);  
}

char* pPtr;
int   gl_integer;

int main(int argc, char* argv[])
{
  struct rusage ru;
  char* p = NULL;


  printf ("printf:      0x%08x\n", printf);
  printf ("show_rusage: 0x%08x\n", show_rusage);
  memset (&ru, 0, sizeof(ru));

  void *brk = sbrk(0);
  printf ("brk: 0x%08x\n", brk);

  printf ("ptr:   0x%08x\n", pPtr);
  printf ("gl_integer:   %d\n", gl_integer);

  for (int i=0; i<1; ++i) {
    p = (char* ) malloc (4096*1024);
    if (p) {
      memset (p, 0x61, 4096*1000);
    }
    else 
      break;
    printf ("p's address : 0x%08x\n", p);

    //    for (int j=0; j<500000000; ++j)
    //      ;
    getrusage(RUSAGE_SELF, &ru);
    //    printf ("%-04d usage: \n", i);
    //    show_rusage(&ru);
  }
  pause();

  return 0;
}
