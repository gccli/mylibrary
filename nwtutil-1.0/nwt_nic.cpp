#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "nwt_nic.h"

int nwt_Nic::usock = -1;
nwt_Nic::nwt_Nic()
{
}

nwt_Nic::~nwt_Nic()
{
	deinit();
}

int nwt_Nic::init()
{ 
	if (usock > 0) return 0;

	usock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (usock < 0) {
		fprintf (stderr, "socket() error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	return 0;
}

void nwt_Nic::deinit()
{
	if (usock > 0) {
		close (usock);
		usock = -1;
	}
}

int nwt_Nic::get_ipaddr(const char* ifname, char * ipaddr) 
{
	init();

	struct ifreq ifr;
	memset (&ifr, 0, sizeof (ifr));
	strcpy (ifr.ifr_name, ifname);
	
	if (ioctl (usock, SIOCGIFADDR, &ifr) < 0) {
		fprintf (stderr, "ioctl(SIOCGIFADDR) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	strcpy (ipaddr, inet_ntoa(((struct sockaddr_in* ) &(ifr.ifr_addr))->sin_addr));

	deinit();
	return 0;

}

int nwt_Nic::get_hwaddr(const char* ifname, char* hwaddr)
{
	init();
	struct ifreq ifr;
	memset (&ifr, 0, sizeof (ifr));
	strcpy (ifr.ifr_name, ifname);
	if (ioctl (usock, SIOCGIFHWADDR, &ifr) < 0) {
		fprintf (stderr, "ioctl(SIOCGIFHWADDR) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}
	memcpy (hwaddr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	deinit();
	return 0;
}

int nwt_Nic::set_ifpromisc(const char* ifname, bool enable)
{
	init ();
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
		fprintf (stderr, "ioctl(SIOCSIFFLAGS) error: %d (%s)\n", errno, strerror(errno));
		ret = 1;
	}
	}
	else ret = 1;

	deinit();
	return ret;
}

