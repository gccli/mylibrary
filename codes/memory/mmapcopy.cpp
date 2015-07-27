#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>


int main(int argc, char* argv[])
{
  if (argc < 2) {
    printf ("usage: %s srcfile [dstfile]\n", argv[0]);
    return 1;
  }
  const char* srcfile = argv[1];
  const char* dstfile = argv[2];
  int fd[2];
  int length, len;
  void *mem = NULL;
  struct stat st;

  // open source file
  if ((fd[0] = open (srcfile, O_RDONLY, 0600)) < 0) {
    fprintf (stderr, "open() error: %d %s\n", errno, strerror(errno));
    return 1;
  }
  
  // get source file lengthgth
  if (fstat (fd[0], &st) < 0) {
    fprintf (stderr, "stat() error: %d %s\n", errno, strerror(errno));
    return 1;
  }
  length  = st.st_size;

  // map source file in virtual memory
  if ((mem = mmap (0, length, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd[0], 0)) == MAP_FAILED) {
    fprintf (stderr, "mmap() error: %d %s\n", errno, strerror(errno));
    return 1;
  }

  // open and write destination file
  if (dstfile == NULL || *dstfile == 0) {
    fd[1] = STDOUT_FILENO;
  }
  else if ((fd[1] = open (dstfile, O_RDWR|O_CREAT, 0600)) < 0) {
    fprintf (stderr, "open() error: %d %s\n", errno, strerror(errno));
    return 1;
  }
  if ((len=write (fd[1], mem, length)) < 0) {
    fprintf (stderr, "write() error: %d %s\n", errno, strerror(errno));
    return 1;
  }

  close (fd[0]);
  close (fd[1]);
  munmap(mem, length);

  return 0;
}
