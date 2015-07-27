#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/shm.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>
#include <semaphore.h>

#include <dirent.h>

static char filename[128];

int LookupFile(const char *filename)
{
  const char *self = "/proc/self/fd";

  DIR *dirp;
  struct dirent *dp;

  char fullpath[256] = {0};
  char linkname[256] = {0};

  if ((dirp = opendir(self)) == NULL)
  {
    printf("failed to open dir: %s\n", strerror(errno));
    return 1;
  }

  do {
    errno = 0;
    if ((dp = readdir(dirp)) != NULL) {
      if (dp->d_name[0] == '.') continue ;

      snprintf(fullpath, sizeof(fullpath), "%s/%s", self, dp->d_name);
      readlink(fullpath, linkname, sizeof(linkname));
      printf("%s -> %s\n", dp->d_name, linkname);
      if (strcmp(linkname, filename))
	continue ;
      printf("found %s, fd is %s\n", filename, dp->d_name);
      closedir(dirp);
      return 0;
    }
  } while(dp);
  if (errno)
    perror("error reading directory");
  else
    printf("failed to find %s\n", filename);
  closedir(dirp);

  return 1;
}

int LockFile(int fd)
{
  struct flock lock;
  memset(&lock, 0, sizeof(lock));
  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;

  if (fcntl(fd, F_SETLK, &lock))
  {
    printf("F_SETLK failed:%s\n", strerror(errno));
    memset(&lock, 0, sizeof(lock));
    if (fcntl(fd, F_GETLK, &lock) == 0)
    {
      printf("'%s' has been lock by process %d\n", filename, lock.l_pid);
    }
    return 1;
  }  
  else {
    puts("file has not been opened");
  }

  return 0;
}

void *ThreadX(void *param)
{
  pthread_detach(pthread_self());

  //;  CheckFile(filename);
  LookupFile(filename);
  int fd = open(filename, O_WRONLY|O_CREAT);
  if (fd < 0) {
    perror("open");
    return NULL;
  }
  if (LockFile(fd))
  {
    close(fd);
    return NULL;
  }
  else
  {
    printf("Thread<%d> lock file '%s'\n", pthread_self(), filename);
    sleep(60);
  }
  
  return NULL;
}

int main(int argc, char *argv[])
{
  if (argc < 2)
  {
    printf("usage: %s filename\n", argv[0]);
    return 0;
  }
  strcpy(filename, argv[1]);

  CheckFile(filename);


  int fd = open(filename, O_WRONLY|O_CREAT);
  if (fd < 0) {
    perror("open");
    return 1;
  }
  if (LockFile(fd))
  {
    close(fd);
    return 1;
  }

  int c;
  while ((c = getchar()))
  {
    if (c == 'c') {
      pthread_t th;
      pthread_create(&th, NULL, ThreadX, NULL);
    }
    else if (c == 'x') {
      close(fd);
    }
    else if (c == 'q') {
      break;
    }
  }

  return 0;
}
