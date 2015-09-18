#ifndef UTIL_SOCK_H__
#define UTIL_SOCK_H__

#include "globaldefines.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

extern void setrcvbuflen(int sockfd, int buflen);
extern void setsndbuflen(int sockfd, int buflen);
extern void setsndtimeout(int sockfd, int sec);
extern void setrcvtimeout(int sockfd, int sec);

extern int  getrcvbuflen(int sockfd);
extern int  getsndbuflen(int sockfd);
extern int getMSS(int sockfd);
extern int setreuse(int sockfd);

static inline int sockfd_to_family(int sockfd)
{
    struct sockaddr_storage ss;
    socklen_t len = sizeof(ss);
    if (getsockname(sockfd, (struct sockaddr *) &ss, &len) < 0)
        return -1;

    return(ss.ss_family);
}

// use for setsockopt & getsockopt
static inline int family_to_level(int family)
{
    switch (family) {
        case AF_INET:
            return IPPROTO_IP;
        case AF_INET6:
            return IPPROTO_IPV6;
        default:
            return -1;
    }
}


static inline int set_nonblocking(int sock, int on)
{
    int flags = 0;
    if ((flags = fcntl (sock, F_GETFL, 0)) < 0) {
        fprintf(stderr, "fcntl(F_GETFL) error: %d (%s)\n",
                errno, strerror(errno));
        return 1;
    }

    if (on)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;

    if ((flags = fcntl (sock, F_SETFL, flags)) < 0) {
        fprintf(stderr, "fcntl(F_SETFL) error: %d (%s)\n",
                errno, strerror(errno));
        return 1;
    }

    return 0;
}

// nonblocking write and read, timeout is 3 second
extern int nonblockwrite(int fd, char *ptr, int nbytes);
extern int nonblockwrite_ext(int fd, char *ptr, int nbytes, char *errmsg);

extern int readn(int fd, char *ptr, int nbytes);
extern ssize_t recvfromext(int fd, void *ptr, size_t nbytes, int *flagsp,
struct sockaddr *sa, socklen_t *salenptr, struct sockaddr_storage *dstp);

// for ip multicast
extern int mcast_join(int sockfd, const struct sockaddr *grp,
                      socklen_t grplen, const char *ifname, int ifindex);
extern int mcast_leave(int sockfd, const struct sockaddr *grp,
                       socklen_t grplen);
extern int mcast_block_source(int sockfd, const struct sockaddr *src,
                              socklen_t srclen, const struct sockaddr *grp,
                              socklen_t grplen);
extern int mcast_unblock_source(int sockfd, const struct sockaddr *src,
                                socklen_t srclen, const struct sockaddr *grp,
                                socklen_t grplen);
extern int mcast_join_source_group(int sockfd, const struct sockaddr *src,
                                   socklen_t srclen, const struct sockaddr *grp,
                                   socklen_t grplen, const char *ifname,
                                   u_int ifindex);
extern int mcast_leave_source_group(int sockfd, const struct sockaddr *src,
                                    socklen_t srclen,
                                    const struct sockaddr *grp,
                                    socklen_t grplen);
extern int mcast_set_if(int sockfd, const char *ifname, u_int ifindex);
extern int mcast_set_loop(int sockfd, int flag);
extern int mcast_set_ttl(int sockfd, int ttl);

#endif
