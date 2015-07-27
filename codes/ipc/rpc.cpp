#include <unistd.h>
#include <stdio.h>
#include <rpc/rpc.h>
#include <stdlib.h>
#include <utmp.h>
#include <rpcsvc/rusers.h>

int main(int argc, char * argv[])
{
  if (argc < 2) {
    printf ("usage: rpc remotehost.\n");
    return 1;
  }
  int num,  nusers;
  clnt_stat stat;
  const char* remotehost = argv[1];

  if (stat = callrpc(argv[1],
		     RUSERSPROG, RUSERSVERS, RUSERSPROC_NUM,
		     xdr_void, 0, xdr_u_long, &nusers) != 0) {
    clnt_perrno(stat);
    exit(1);
  }
  printf("%d users on %s\n", nusers, argv[1]);

  //  if ((num = rnusers(remotehost)) < 0) {
  //    perror ("runusers");
  //    return 1;
  //  }
  printf ("%d users running on %s", num, remotehost);

  while (true) {
    sleep (1);
  }
  return 0;
}
