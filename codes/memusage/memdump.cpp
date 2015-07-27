#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <assert.h>
#include <signal.h>

#include "logger.h"

static int verbose = 1;

void wait_until_tracee_stops(pid_t pid)
{
   int   status = 0;
   pid_t ret;
   while (true)
   {
      ret = waitpid(pid, &status, 0);
      if (verbose > 0) printf("waitpid %d return status %d\n", ret, status);
      if (ret != pid)
      {
	 perror("waitpid()");
	 exit(1);
      }
      if (!WIFSTOPPED(status))
      {
	 fprintf(stderr, "Unhandled status change: %d\n", status);
	 exit(1);
      }
      if (WSTOPSIG(status) == SIGSTOP)
	 break;
      int signal = WSTOPSIG(status);
      fprintf(stderr, "Passing signal to child: %d\n", signal);
      if (ptrace(PTRACE_CONT, pid, NULL, WSTOPSIG(status)) != 0)
      {
	 perror("ptrace(PTRACE_CONT)");
	 exit(1);
      }
   }
}

int main(int argc, char *argv[])
{
   if (argc < 4)
   {
      fprintf(stderr, "Usage: %s pid address size\n", argv[0]);
      return 2;
   }

   pid_t const pid = atoi(argv[1]);
   off_t const address = strtoul(argv[2], NULL, 16);
   size_t const size = strtoul(argv[3], NULL, 0);

   /* prepare for memory reads */
   char *mempath = NULL;
   asprintf(&mempath, "/proc/%d/mem", pid);
   char *outbuf = (char *)malloc(size);
   assert(outbuf != NULL);

   // attach or exit with code 3
   if (0 != ptrace(PTRACE_ATTACH, pid, NULL, NULL))
   {
      printf("ptrace attach error - %s\n", strerror(errno));
      return -1;
   }

   wait_until_tracee_stops(pid);
   int memfd = open(mempath, O_RDONLY);
   assert(memfd > 0);

   // read bytes from the tracee's memory
   int len = pread(memfd, outbuf, size, address);
   if (len < 0) {
      printf("pread error - %s\n", strerror(errno));
   } else if (verbose > 0) {
      printf("%d bytes of memory read\n", len);
      PrintPayload((unsigned char *)outbuf, len);
   }
   close(memfd);
   ptrace(PTRACE_DETACH, pid, NULL, 0);

   free(outbuf);
   free(mempath);

   return 0;
}
