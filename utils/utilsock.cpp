#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <net/if.h>
#include <sys/ioctl.h>

#include "globaldefines.h"
#include "utilsock.h"

int nonblockwrite(int fd, char *ptr, int nbytes)
{
  int  inum = 0;
  char star[] = {'/','-','\\','|'};

  set_nonblocking(fd, 1);

  int nleft, nwritten, nready;
  fd_set fset;
  struct timeval tv;

  nleft = nbytes;
  while (nleft > 0) {
    tv.tv_sec = 15; tv.tv_usec = 0;
    FD_ZERO(&fset);
    FD_SET(fd, &fset);
    if((nready = select(fd+1, NULL, &fset, NULL, &tv)) == 0) {
      errno = ETIMEDOUT;
      fprintf(stderr, FMTRED"fd %d timeout\n"FMTEND, fd);
      break;
    }
    else if (nready < 0) {
      if(errno == EINTR)
        continue;
      fprintf(stderr, FMTRED"select error, %d (%s)\n"FMTEND, errno, strerror(errno));
      return -1;
    }

    nwritten = send(fd, ptr, nleft, MSG_NOSIGNAL);
    if (nwritten <= 0) {
      if(errno == EAGAIN || errno == EINTR)
        continue;
      fprintf(stderr, FMTRED"write error, %d (%s)\n"FMTEND, errno, strerror(errno));
      return -1;
    }

    nleft -= nwritten;
    ptr   += nwritten;
    fprintf(stderr, TCKL"%2c%8d bytes written, %d/%d bytes data sent ", star[inum++%4], nwritten, nbytes-nleft, nbytes);
  }
  fprintf(stderr, TCKL);

  set_nonblocking(fd, 0);

  return (nbytes - nleft);
}


int nonblockwrite_ext(int fd, char *ptr, int nbytes, char *errmsg)
{
  int  inum = 0;
  char star[] = {'/','-','\\','|'};

  int erroccur = 0;
  int nleft, nwritten, nsend, nready, timeout = 15;
  const int slice_length = 1024;
  if (nbytes <= slice_length)
    return send(fd, ptr, nbytes, MSG_NOSIGNAL);

  set_nonblocking(fd, 1);

  fd_set fset;
  struct timeval tv;

  nleft = nbytes;
  nsend = slice_length;
  while (nleft > 0) {
// polling
    tv.tv_sec = timeout; tv.tv_usec = 0;
    FD_ZERO(&fset);
    FD_SET(fd, &fset);
    if((nready = select(fd+1, NULL, &fset, NULL, &tv)) == 0) {
      errno = ETIMEDOUT;
      sprintf(errmsg, "sockfd:%d timeout", fd);
      break;
    }
    else if (nready < 0) {
      if(errno == EINTR)
        continue;
      sprintf(errmsg, "sockfd:%d select error, %d (%s)", fd, errno, strerror(errno));
      erroccur = 1;
      break;
    }

// send data
    if (nleft < slice_length)
      nsend = nleft;

    nwritten = send(fd, ptr, nsend, MSG_NOSIGNAL);
    if (nwritten <= 0) {
      if(errno == EAGAIN || errno == EINTR)
        continue;
      sprintf(errmsg, "sockfd:%d send error, %d (%s)", fd, errno, strerror(errno));
      erroccur = 1;
      break;
    }

    nleft -= nwritten;
    ptr   += nwritten;
    fprintf(stderr, TCKL"%2c%8d bytes written, %d/%d bytes data sent ", star[inum++%4], nwritten, nbytes-nleft, nbytes);
  }
  fprintf(stderr, TCKL);

  set_nonblocking(fd, 0);

  if (erroccur)
    return -1;

  return (nbytes - nleft);
}


int readn(int fd, char *ptr, int nbytes)
{
  int  inum = 0;
  char star[] = {'/','-','\\','|'};

  int nread, nready, nleft = nbytes;

  fd_set fset;
  struct timeval tv;

  while (nleft > 0) {
    tv.tv_sec = 5; tv.tv_usec = 0;
    FD_ZERO(&fset);
    FD_SET(fd, &fset);
    if((nready = select(fd+1, NULL, &fset, NULL, &tv)) == 0) {
      errno = ETIMEDOUT;
      fprintf(stderr, FMTRED"fd %d timeout\n"FMTEND, fd);
      break;
    }
    else if (nready < 0) {
      if(errno == EINTR)
        continue;
      fprintf(stderr, FMTRED"select error, %d (%s)\n"FMTEND, errno, strerror(errno));
      return -1;
    }

    if ((nread = recv(fd, ptr, nleft, 0)) < 0) {
      if (errno == EINTR)
        continue;
      fprintf(stderr, FMTRED"read error, %d (%s)\n"FMTEND, errno, strerror(errno));
      return -1;
    }
    else if (nread == 0) {
      fprintf(stderr, FMTRED"fd %d EOF\n"FMTEND, fd);
      break;                                      // EOF
    }

    nleft -= nread;
    ptr   += nread;
    fprintf(stderr, TCKL"%2c%8d bytes read, %d/%d bytes data received ", star[inum++%4], nread, nbytes-nleft, nbytes);
  }
  fprintf(stderr, TCKL);

  return (nbytes - nleft);
}


ssize_t recvfromext(int fd, void *ptr, size_t nbytes, int *flagsp,
struct sockaddr *sa, socklen_t *salenptr, struct sockaddr_storage *dstp)
{
  struct msghdr msg;
  struct iovec  iov[1];

  unsigned char control[CMSG_SPACE(sizeof(struct sockaddr_storage))];
  msg.msg_control = control;
  msg.msg_controllen = sizeof(control);
  msg.msg_flags = 0;
  msg.msg_name = sa;
  msg.msg_namelen = *salenptr;
  iov[0].iov_base = ptr;
  iov[0].iov_len = nbytes;
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

  ssize_t n;
  if ((n = recvmsg(fd, &msg, *flagsp)) < 0)
    return(n);

  *salenptr = msg.msg_namelen;
  if (dstp)
    bzero(dstp, sizeof(struct sockaddr_storage));

  *flagsp = msg.msg_flags;                        /* pass back results */
  if (msg.msg_controllen < sizeof(struct cmsghdr) || (msg.msg_flags & MSG_CTRUNC) || dstp == NULL)
    return(n);
  struct cmsghdr *cmptr = CMSG_FIRSTHDR(&msg);
  for (; cmptr != NULL; cmptr = CMSG_NXTHDR(&msg, cmptr)) {
    if (cmptr->cmsg_level == IPPROTO_IPV6 && cmptr->cmsg_type == IPV6_PKTINFO) {
      struct in6_pktinfo *in6 = (struct in6_pktinfo *) CMSG_DATA(cmptr);
      struct sockaddr_in6 *a6 = (struct sockaddr_in6 *) dstp;
      a6->sin6_family = AF_INET6;
      memcpy(&a6->sin6_addr, &in6->ipi6_addr, sizeof(struct in6_addr));
      printf("From interface %d\n", in6->ipi6_ifindex);
      continue;
    }
    if (cmptr->cmsg_level == IPPROTO_IP && cmptr->cmsg_type == IP_PKTINFO) {
      struct in_pktinfo *in4 = (struct in_pktinfo *) CMSG_DATA(cmptr);
      struct sockaddr_in *a4 = (struct sockaddr_in *) dstp;
      a4->sin_family = AF_INET;
      memcpy(&a4->sin_addr, &in4->ipi_spec_dst, sizeof(struct in_addr));
      printf("From interface %d\n", in4->ipi_ifindex);
      continue;
    }
    printf("unknown ancillary data, len = %zu, level = %d, type = %d\n",
      cmptr->cmsg_len, cmptr->cmsg_level, cmptr->cmsg_type);
  }
  return(n);
}


int mcast_join(int sockfd, const struct sockaddr *grp, socklen_t grplen, const char *ifname, int ifindex)
{
  struct group_req req;
  if (ifindex > 0) {
    req.gr_interface = ifindex;
  }
  else if (ifname != NULL) {
    if ((req.gr_interface = if_nametoindex(ifname)) == 0) {
      return -1;
    }
  }
  else
    req.gr_interface = 0;
  if (grplen > sizeof(req.gr_group)) {
    return -1;
  }

  memcpy(&req.gr_group, grp, grplen);

  return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_JOIN_GROUP, &req, sizeof(req));;
}


int mcast_leave(int sockfd, const struct sockaddr *grp, socklen_t grplen)
{
  struct group_req req;
  req.gr_interface = 0;
  if (grplen > sizeof(req.gr_group)) {
    return -1;
  }
  memcpy(&req.gr_group, grp, grplen);
  return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_LEAVE_GROUP, &req, sizeof(req));
}


int mcast_block_source(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen)
{
  struct group_source_req req;
  req.gsr_interface = 0;
  if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
    return -1;
  }

  memcpy(&req.gsr_group, grp, grplen);
  memcpy(&req.gsr_source, src, srclen);
  return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_BLOCK_SOURCE, &req, sizeof(req));
}


int mcast_unblock_source(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen)
{
  struct group_source_req req;
  req.gsr_interface = 0;
  if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
    errno = EINVAL;
    return -1;
  }

  memcpy(&req.gsr_group, grp, grplen);
  memcpy(&req.gsr_source, src, srclen);
  return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_UNBLOCK_SOURCE, &req, sizeof(req));
}


int mcast_join_source_group(int sockfd, const struct sockaddr *src, socklen_t srclen,
const struct sockaddr *grp, socklen_t grplen, const char *ifname, u_int ifindex)
{
  struct group_source_req req;
  if (ifindex > 0) {
    req.gsr_interface = ifindex;
  }
  else if (ifname != NULL) {
    if ((req.gsr_interface = if_nametoindex(ifname)) == 0) {
      return -1;
    }
  }
  else
    req.gsr_interface = 0;

  if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
    return -1;
  }
  memcpy(&req.gsr_group, grp, grplen);
  memcpy(&req.gsr_source, src, srclen);

  return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_JOIN_SOURCE_GROUP, &req, sizeof(req));;
}


int mcast_leave_source_group(int sockfd, const struct sockaddr *src, socklen_t srclen, const struct sockaddr *grp, socklen_t grplen)
{
  struct group_source_req req;
  req.gsr_interface = 0;
  if (grplen > sizeof(req.gsr_group) || srclen > sizeof(req.gsr_source)) {
    return -1;
  }

  memcpy(&req.gsr_group, grp, grplen);
  memcpy(&req.gsr_source, src, srclen);
  return setsockopt(sockfd, family_to_level(grp->sa_family), MCAST_LEAVE_SOURCE_GROUP, &req, sizeof(req));
}


int mcast_set_if(int sockfd, const char *ifname, u_int ifindex)
{
  int type = sockfd_to_family(sockfd);
  switch (type) {
    case AF_INET:
    {
      struct in_addr inaddr;
      struct ifreq   ifreq;
      if (ifindex > 0) {
        if (if_indextoname(ifindex, ifreq.ifr_name) == NULL) {
          return(-1);
        }
        goto doioctl;
      }
      else if (ifname != NULL) {
        strncpy(ifreq.ifr_name, ifname, IFNAMSIZ);
        doioctl:
        if (ioctl(sockfd, SIOCGIFADDR, &ifreq) < 0)
          return -1;
        memcpy(&inaddr, &((struct sockaddr_in *) &ifreq.ifr_addr)->sin_addr, sizeof(struct in_addr));
      }
      else
        inaddr.s_addr = htonl(INADDR_ANY);

      return setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &inaddr, sizeof(struct in_addr));
    }
    case AF_INET6:
    {
      u_int idx;
      if ((idx = ifindex) == 0) {
        if (ifname == NULL) {
          return(-1);
        }
        if ((idx = if_nametoindex(ifname)) == 0) {
          return(-1);
        }
      }
      return setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &idx, sizeof(idx));
    }
    default:
      return(-1);
  }
}


int mcast_set_loop(int sockfd, int flag)
{
  switch (sockfd_to_family(sockfd)) {
    case AF_INET:
      return(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, (u_char *)&flag, sizeof(u_char)));
    case AF_INET6:
      return(setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (u_int *) &flag, sizeof(u_int)));
    default:
      return(-1);
  }
}


int mcast_set_ttl(int sockfd, int val)
{
  switch (sockfd_to_family(sockfd)) {
    case AF_INET:
      return(setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, (u_char *)&val, sizeof(u_char)));
    case AF_INET6:
      return(setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (int *) &val, sizeof(int)));
    default:
      return(-1);
  }
}
