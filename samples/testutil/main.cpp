#include "utilnet.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

char *print_hwaddr(const unsigned char *hwaddr, int len)
{
	static char buffer[64];
	int buflen = 0, tmplen;

	tmplen = sprintf(buffer+buflen, "%02x", hwaddr[0]);
	buflen += tmplen;
	for (int i=1; i<len; ++i)
	{
		tmplen = sprintf(buffer+buflen, ":%02x", hwaddr[i]);	
		buflen += tmplen;
	}

	return buffer;
}

int test_utilnet(int argc, char* argv[])
{
	const char *ifname = argv[1];
	if (ifname == NULL)
		ifname = "eth0";

	char ipaddr[16], macaddr[6];

	util_get_ipaddr(ifname, ipaddr);
	util_get_hwaddr(ifname, macaddr);
	
	printf("Interface   : %s\n"
		   "IP Address  : %s\n"
		   "MAC Address : %s\n", ifname, ipaddr, print_hwaddr((unsigned char *)macaddr, sizeof(macaddr)));

	return 0;
}

int main(int argc, char* argv[])
{
	return test_utilnet(argc, argv);
}

