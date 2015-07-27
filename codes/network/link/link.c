#define _GNU_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <netdb.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>

#include <pthread.h>
#include <getopt.h>
#define MAX_IFS 64

int main(int argc, char *argv)
{
	struct sockaddr *addr = NULL;

	int i;
	int sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

	struct ifconf ifc;
	struct ifreq  ifs[MAX_IFS];
	struct ifreq  req;
	struct ifreq *ifr, *ifend;
	memset(&ifc, 0, sizeof(ifc));

	ifc.ifc_len = sizeof(ifs);
	ifc.ifc_req = ifs;

	if (ioctl(sock, SIOCGIFCONF, &ifc) != 0)
	{
		fprintf(stderr, "IOCTL SIOCGIFCONF %s\n", strerror(errno));
		return 1;
	}

	ifend = ifs + (ifc.ifc_len / sizeof(struct ifreq));
	for (ifr = ifc.ifc_req; ifr < ifend; ifr++)
	{
		if (ifr->ifr_addr.sa_family == AF_INET)
		{
			strncpy(req.ifr_name, ifr->ifr_name,sizeof(req.ifr_name));
			if (ioctl (sock, SIOCGIFHWADDR, &req) < 0)
			{
			  printf("IOCTL SIOCGIFHWADDR(%s): %m\n", req.ifr_name);
			  return 0;
			}
			printf("IP %s\n", inet_ntoa( ( (struct sockaddr_in *)  &ifr->ifr_addr)->sin_addr));
			addr = (struct sockaddr *) &req.ifr_hwaddr;
			for (i=0; i<6; ++i) {
				if (i == 5)
					printf("%02x\n", (unsigned char )addr->sa_data[i]);
				else
					printf("%02x:", (unsigned char )addr->sa_data[i]);
			}
		}
	}

	if (ioctl(sock, SIOCGIFMTU, &req) != 0)
	{
		fprintf(stderr, "IOCTL SIOCGIFMTU %s\n", strerror(errno));
		return 1;
	}
	printf("MTU %d\n", req.ifr_mtu);

	return 0;
}

