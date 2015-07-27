#ifndef IPV6_CONFIG_H__
#define IPV6_CONFIG_H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>

#include <net/if.h>

#include <netinet/icmp6.h>
#include <netinet/ip6.h>
#include <netinet/ip_icmp.h>
#include <linux/types.h>
#include <linux/errqueue.h>


struct ipv6_config {
	int         verbose;   // -v: verbose mode
	int         count;     // -c: repeat count
	int         ttl;       // -t: set ttl
	int         debug;     // -D: debug mode

	char        ifname[IF_NAMESIZE]; // -I interface address: Set source address to specified interface address.  Argument may be numeric IP address or name of device.
	char        command[16];
	char        host[128];
};

extern struct ipv6_config config;
void ipv6_command_line(int argc, char *argv[]);


enum IPV6_OPTION {
	F_RROUTE	      =0x0001,
	F_SO_DEBUG	      =0x0002,
	F_SO_DONTROUTE    =0x0080,
	F_TIMESTAMP       =0x0100,
	F_FLOWINFO        =0x0200,
};



#endif

