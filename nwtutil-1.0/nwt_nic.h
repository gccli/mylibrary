#ifndef NWT_NIC_H_
#define NWT_NIC_H_

#include <unistd.h>
#include <net/if.h>
#include <netinet/if_ether.h>

class nwt_Nic {
public:
	nwt_Nic();
	~nwt_Nic();
	
public:
	static int	get_ipaddr(const char* ifname, char* hwaddr);
	static int  get_hwaddr(const char* ifname, char* hwaddr);
	static int  set_ifpromisc(const char* ifname, bool bset = true);
	static int  init();
	static void deinit();

private:
	static int usock;
};

#endif // NWT_NIC_H_

