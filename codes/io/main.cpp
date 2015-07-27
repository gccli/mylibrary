#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  if (argc < 2)
    return 1;
  int fd = open (argv[1], O_RDWR|O_CREAT);
  if (fd < 0) {
    perror ("open");
    return 1;
  }
  
  off_t start = 0;
  off_t end = lseek(fd, start, SEEK_END);
  lseek (fd, start, SEEK_SET);

  int ch=0;
  if (argc == 3) ch = atoi (argv[2]);
  
  if (ch == 1)
    posix_fadvise (fd, start, end, POSIX_FADV_SEQUENTIAL);
  if (ch == 2)
    posix_fadvise (fd, start, end, POSIX_FADV_RANDOM);

  if (readahead (fd, start, end) < 0)
    perror ("readahead");


  struct timeval tvstart, tvend;
  gettimeofday (&tvstart, NULL);
  
  int len, totallen=0;
  char buffer[1024];
  while ((len = read (fd, buffer, sizeof(buffer))) > 0) {
    totallen += len;
    //    printf ("read len = %d\n", len);
  }
  printf ("total length = %d\n", totallen);
  gettimeofday (&tvend, NULL);
  double tv = (tvend.tv_sec - tvstart.tv_sec) + 1.0*(tvend.tv_usec - tvstart.tv_usec)/1000000;

  printf ("time cost: %lf\n", tv);

  return 0;
}
