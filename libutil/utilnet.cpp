#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <net/if.h>
#include <netinet/if_ether.h>

#include <sys/ioctl.h>
#include "globaldefines.h"
#include "utilsock.h"

int util_get_ipaddr(const char* ifname, char * ipaddr)
{
  int usock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (usock < 0) {
    fprintf (stderr, "socket() error: %d (%s)\n", errno, strerror(errno));
    return 1;
  }

  struct ifreq ifr;
  memset (&ifr, 0, sizeof (ifr));
  strcpy (ifr.ifr_name, ifname);

  if (ioctl (usock, SIOCGIFADDR, &ifr) < 0) {
    fprintf (stderr, "ioctl(SIOCGIFADDR) error: %d (%s)\n",
             errno, strerror(errno));
    return 1;
  }
  strcpy (ipaddr, inet_ntoa(((struct sockaddr_in*) &(ifr.ifr_addr))->sin_addr));

  close(usock);
  return 0;

}

int util_get_hwaddr(const char* ifname, char* hwaddr)
{
  int usock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (usock < 0) {
    fprintf (stderr, "socket() error: %d (%s)\n", errno, strerror(errno));
    return 1;
  }

  struct ifreq ifr;
  memset (&ifr, 0, sizeof (ifr));
  strcpy (ifr.ifr_name, ifname);
  if (ioctl (usock, SIOCGIFHWADDR, &ifr) < 0) {
    fprintf (stderr, "ioctl(SIOCGIFHWADDR) error: %d (%s)\n",
             errno, strerror(errno));
    return 1;
  }
  memcpy (hwaddr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

  close(usock);
  return 0;
}

int util_set_ifpromisc(const char* ifname, bool enable)
{
  int usock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if (usock < 0) {
    fprintf (stderr, "socket() error: %d (%s)\n", errno, strerror(errno));
    return 1;
  }

  int ret = 0;
  struct ifreq ifr;
  memset (&ifr, 0, sizeof (ifr));
  strcpy (ifr.ifr_name, ifname);

  if (ioctl(usock, SIOCGIFFLAGS, &ifr) == 0) {
    if (enable)
      ifr.ifr_flags |= IFF_PROMISC;
    else
      ifr.ifr_flags &= ~IFF_PROMISC;
    if (ioctl(usock, SIOCSIFFLAGS, &ifr) < 0) {
      fprintf (stderr, "ioctl(SIOCSIFFLAGS) error: %d (%s)\n",
               errno, strerror(errno));
      ret = 1;
    }
  }
  else ret = 1;

  close(usock);
  return ret;
}
