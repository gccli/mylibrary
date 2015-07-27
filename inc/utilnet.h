#ifndef UTIL_NET_H_
#define UTIL_NET_H_

extern int util_get_ipaddr(const char* ifname, char * ipaddr);
extern int util_get_hwaddr(const char* ifname, char* hwaddr);
extern int util_set_ifpromisc(const char* ifname, bool enable);

#endif
