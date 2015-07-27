/**
 *  void *shmat(int shmid, const void *shmaddr, int shmflg);
 *  int shmdt(const void *shmaddr);
 *  int shmget(key_t key, size_t size, int shmflg);
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>

#define SHMSZ     27
#define gettid() syscall(__NR_gettid)

static key_t get_shm_key(void)
{
  /* both arbitrary */
  const char * KEY_PATH = "/dev/null";
  const char   KEY_ID   = 0x64;

  key_t key = ftok(KEY_PATH, KEY_ID);
  if (key == (key_t)-1)
    perror("ftok");

  return key;
}

int get_shm_min(void)
{
  struct shminfo info;
  if((shmctl(0, IPC_INFO, (struct shmid_ds *)(void *)&info)) == -1)
    perror("shmctl (shminfo)");
  return info.shmmin;
}

int childproc()
{
  key_t key = get_shm_key();
  printf("key: %u\n", key);

  int shmid;
  if ((shmid = shmget(key, SHMSZ, 0666)) < 0) {
    perror("shmget");
    exit(1);
  }
  printf("shmid: %u\n", shmid);

  char *shm;
  if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }

  return 0;
}


int main(int argc, char* argv[])
{
  int childproc = 0;

  pid_t pid;
  if ((pid = fork()) == 0) {
    sleep(1); // parent process run first
    childproc = 1;
    putchar('\n');
  }
  
  key_t key = get_shm_key();
  printf("key: %u\n", key);

  int shmid;
  if ((shmid = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
    perror("shmget");
    exit(1);
  }
  printf("shmid: %u\n", shmid);

  char *shm;
  if ((shm = shmat(shmid, NULL, 0)) == (char *) -1) {
    perror("shmat");
    exit(1);
  }

  char c;
  char *s = shm;
  if (!childproc) {
    for (c = 'a'; c <= 'z'; c++) 
      *s++ = c;
    *s = 0;
    while (*shm != '*')
      ;
  }
  else {
    for (s = shm; *s != 0; s++)
      putchar(*s);
    putchar('\n');
    *shm = '*';
  }

  return 0;
}
