#include <sys/stat.h>
#include <fcntl.h>

#include "globaldefines.h"
#include "utilsock.h"

void setrcvbuflen(int sockfd, int buflen)
{
  int size = 0, newsize = 0;
  socklen_t len = sizeof(int);
  getsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, &size, &len);
  setsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, &buflen, sizeof(int));
  getsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, &newsize, &len);

  printf("RCVBUF %d KB, OLD %d KB\n", newsize/KBytes, size/KBytes);
}


void setsndbuflen(int sockfd, int buflen)
{
  int size = 0, newsize = 0;
  socklen_t len = sizeof(int);
  getsockopt (sockfd, SOL_SOCKET, SO_SNDBUF, &size, &len);
  setsockopt (sockfd, SOL_SOCKET, SO_SNDBUF, &buflen, sizeof(int));
  getsockopt (sockfd, SOL_SOCKET, SO_SNDBUF, &newsize, &len);

  printf("SNDBUF %d KB, OLD %d KB\n", newsize/KBytes, size/KBytes);
}


void setsndtimeout(int sockfd, int sec)
{
  struct timeval tv_timeout = {sec, 0};
  if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO,(void *)&tv_timeout, sizeof(tv_timeout)) < 0) {
    fprintf (stderr, "setsockopt(SO_SNDTIMEO) error: %d (%s)\n", errno, strerror(errno));
  }
}


void setrcvtimeout(int sockfd, int sec)
{
  struct timeval tv_timeout = {sec, 0};
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,(void *)&tv_timeout, sizeof(tv_timeout)) < 0) {
    fprintf (stderr, "setsockopt(SO_RCVTIMEO) error: %d (%s)\n", errno, strerror(errno));
  }
}


int getMSS(int sock)
{
  int mss = 0;
  socklen_t len = sizeof(int);
  if (getsockopt (sock, IPPROTO_TCP, TCP_MAXSEG, &mss, &len) < 0) {
    fprintf (stderr, "getsockopt(TCP_MAXSEG) error: %d (%s)\n", errno, strerror(errno));
  }

  return mss;
}


int getrcvbuflen(int sockfd)
{
  int size = 0;
  socklen_t len = sizeof(int);
  getsockopt (sockfd, SOL_SOCKET, SO_RCVBUF, &size, &len);

  return size;
}


int getsndbuflen(int sockfd)
{
  int size = 0;
  socklen_t len = sizeof(int);
  getsockopt (sockfd, SOL_SOCKET, SO_SNDBUF, &size, &len);

  return size;
}


int setreuse(int sock)
{
  int reuse = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse, sizeof(reuse)) < 0) {
    fprintf (stderr, "setsockopt(SO_REUSEADDR) error: %d (%s)\n", errno, strerror(errno));
    return 1;
  }

  return 0;
}
