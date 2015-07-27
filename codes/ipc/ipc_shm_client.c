#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct {
  char           domain[64]; // TLD(top level domain)
  unsigned short type;       // Service Type or Protocol Type
  unsigned short reserve;
  char           fqdn[128];  // Fully Qualified Domain Name
  char           srvIp[20];
  char           *pUrl;
} ServerType_t;

int main(int argc, char* argv[])
{
  key_t key = atoi(argv[1]);

  int shmid;
  char *shm, *s;
  
  int shmsz = 20*sizeof(ServerType_t);

  if ((shmid = shmget(key, shmsz, 0666)) < 0) {
    perror("shmget");
    exit(1);
  }

  if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }

  ServerType_t *pServ = (ServerType_t *)shm;
  int i;
  for (i = 0; i<20; ++i, pServ += 1) {
    printf("Domain:%s\n", pServ->domain);
    printf("Type:%d\n", pServ->type);
    printf("FQDN:%s\n", pServ->fqdn);
    printf("URL: %s\n", pServ->pUrl);
  }

  return 0;
}
