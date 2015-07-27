#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdlib.h>
#include <aio.h>
#include <libaio.h>
/*
int main1(int argc, char* argv[])
{
  if (argc < 2)
    return 1;

  struct aiocb aio;

  aio.aio_fildes = open (argv[1], O_RDWR|O_CREAT);
  if (aio.aio_fildes < 0) {
    perror ("open");
    return 1;
  }
  aio.aio_nbytes = 20;
  aio_read (&aio);

  while (aio_error(&aio) != 0)
    continue;

  printf ("aio.aio_nbytes:%d, aio_buf:%s\n", 
	  aio.aio_nbytes, aio.aio_buf);

  return 0;
}
*/

int main(int argc, char* argv[])
{
  if (argc < 2)
    return 1;
  
  int ctx_id;
  io_context_t ctx;
  if ((ctx_id = io_setup (20, &ctx)) < 0) {
    perror ("io_setup");
    return 1;
  }

  return 0;
}
