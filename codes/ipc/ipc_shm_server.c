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
  int shmid;
  key_t key = getpid();
  char *shm, *s;
  
  int shmsz = 20*sizeof(ServerType_t);

  if ((shmid = shmget(key, shmsz, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    exit(1);
  }

  printf("Shared Memory ID: %d\n", shmid);

  if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }
  memset(shm, 0, shmsz);

  ServerType_t *pServ = (ServerType_t *)shm;
  int i;
  for (i = 0; i<20; ++i, pServ += 1) {
   strcpy(pServ->domain, "cclinux.org");
   pServ->type = 16+i%3;
   sprintf(pServ->fqdn, "%d.%s", i, pServ->domain);
   pServ->pUrl = strdup("https://mail.cclinux.org/ews/exchange.asmx");
  }

  pServ = (ServerType_t *)shm;
  while (pServ->pUrl != NULL)
    sleep(1);
  printf("quit");

  return 0;
}
