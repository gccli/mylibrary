#include "nwt_socket.h"
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>

#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <netinet/if_fddi.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include "logger.h"

RAWSocket::RAWSocket(int protocol)
	:nwt_socket(AF_INET, SOCK_RAW, protocol)
{
}

RAWSocket::~RAWSocket()
{
}

int RAWSocket::setHeaderIncl()
{
	const int on = 1;
	if (setsockopt (sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0) {
		fprintf (stderr, "setsockopt(IP_HDRINCL) error: %d (%s)\n", errno, strerror(errno));
		return 1;
	}

	return 0;
}

int RAWSocket::buildRawUDPData(const char* srcip, int srcport, const char* dstip, int dstport, const char* payload, int len)
{
	if (setHeaderIncl() != 0)
	{
		logger::instance()->log_error("setsockopt error %d %s\n", errno, strerror(errno));
		return -1;
	}
	char rawbuf[1024] = "\0";
	struct iphdr          iph;
	struct udphdr         udph;
	struct sockaddr_in    dest;

	memset (&iph, 0, sizeof(iph));
	memset (&udph, 0, sizeof(udph));
	memset (&dest, 0, sizeof(dest));
	dest.sin_family      = AF_INET;
	dest.sin_port        = htons(dstport);
	dest.sin_addr.s_addr = inet_addr(dstip);

	// construct ip header
	udph.source       = htons(srcport);
	udph.dest         = htons(dstport);
	udph.len          = htons(sizeof(udph)+len);
	udph.check        = 0;

	iph.ihl           = 5;
	iph.version       = 4;
	iph.tos           = 0;
	iph.tot_len       = htons(sizeof(iph)+sizeof(udph)+len);
	iph.id            = htons(0x8080);
	iph.frag_off      |= htons(IP_DF); //dont fragment flag
	iph.ttl           = 64;
	iph.protocol      = IPPROTO_UDP;
	iph.check         = 0;//chksum((unsigned short *)rawbuf, sizeof(iph)+sizeof(udph));
	iph.saddr         = inet_addr(srcip);
	iph.daddr         = dest.sin_addr.s_addr;

	int offset = 0;
	memcpy (rawbuf+offset, &iph, sizeof(iph));
	offset += sizeof(iph);
	memcpy (rawbuf+offset, &udph, sizeof(udph));
	offset += sizeof(udph);
	memcpy (rawbuf+offset, payload, len);
	offset += len;

	int slen;
	if((slen = sendto(sock, rawbuf, offset, 0, (struct sockaddr *)&dest, sizeof(dest))) < 0) {
		logger::instance()->log_error("write error %d %s\n", errno, strerror(errno));
		return -1;
	}
	logger::instance()->log_debug("send %d byte data to %s:%d\n", slen, inet_ntoa(dest.sin_addr), dstport);
	
	return slen;
}

/**
 * Calculate TCP checksum
 * To calculate TCP checksum a "pseudo header" is added to the TCP header. This includes:
 * 
 * IP Source Address        4 bytes
 * IP Destination Address   4 bytes
 * TCP Protocol             2 bytes
 * TCP Length               2 bytes
 * 
 * The checksum is calculated over all the octets of the pseudo header, TCP header and data. 
 * If the data contains an odd number of octets a pad, zero octet is added to the end of data.
 * The pseudo header and the pad are not transmitted with the packet. 
 * 
 * In this function 
 * @tcpbuf is an array containing all the octets in the TCP header and data.
 * @tcplen is the length (number of octets) of the TCP header and data.
 * @srcaddr[4] and @dstaddr[4] are the IP source and destination address octets.
*/

unsigned short tcp_sum_calc(unsigned short *srcaddr, unsigned short *dstaddr, unsigned short *tcpbuf, unsigned short tcplen)
{
	unsigned short proto = IPPROTO_TCP;
	long sum;
	int i;
	
	sum = 0;
	// Check if the tcp length is even or odd.  Add padding if odd.
	if((tcplen % 2) == 1){
		tcpbuf[tcplen] = 0;  // empty space in the ip buffer should be 0 anyway.
      	tcplen += 1; // incrase length to make even.
	}
	
	// add the pseudo header
	sum += ntohs(srcaddr[0]);
	sum += ntohs(srcaddr[1]);
	sum += ntohs(dstaddr[0]);
	sum += ntohs(dstaddr[1]);
	sum += tcplen;
	sum += proto;
      
	/* 
	 * calculate the checksum for the tcp header and payload
	 * tcplen represents number of 8-bit bytes, 
	 * we are working with 16-bit words so divide tcplen by 2. 
	 */
	for(i=0; i<(tcplen/2); i++) {
		sum += ntohs(tcpbuf[i]);
	}
    
	// keep only the last 16 bits of the 32 bit calculated sum and add the carries
	sum = (sum & 0xFFFF) + (sum >> 16);
	sum += (sum >> 16);
	
	// Take the bitwise complement of sum
	sum = ~sum;

	return htons(((unsigned short) sum));
}

int RAWSocket::buildTcpSYN(const char* srcip, int srcport, const char* dstip, int dstport)
{
	if (setHeaderIncl() != 0)
	{
		logger::instance()->log_error("setsockopt error %d %s\n", errno, strerror(errno));
		return -1;
	}
	char rawbuf[1024] = "\0";
	struct iphdr          iph;
	struct tcphdr         tcph;
	struct sockaddr_in    dest;

	memset (&iph,  0, sizeof(iph));
	memset (&tcph, 0, sizeof(tcph));
	memset (&dest, 0, sizeof(dest));
	dest.sin_family      = AF_INET;
	dest.sin_port        = htons(dstport);
	dest.sin_addr.s_addr = inet_addr(dstip);

	// construct ip header
	iph.ihl           = 5;
	iph.version       = 4;
	iph.tos           = 0;
	iph.tot_len       = htons(sizeof(iph)+sizeof(tcph));
	iph.id            = htons(0x8080);
	iph.frag_off     |= htons(IP_DF); //dont fragment flag
	iph.ttl           = 64;
	iph.protocol      = IPPROTO_TCP;
	iph.check         = 0;//chksum((unsigned short *)rawbuf, sizeof(iph)+sizeof(udph));
	iph.saddr         = inet_addr(srcip);
	iph.daddr         = dest.sin_addr.s_addr;

	// construct tcp header
	tcph.source       = htons(srcport);
	tcph.dest         = htons(dstport);
	tcph.seq          = random();
	tcph.ack_seq      = 0;
	tcph.doff         = sizeof(tcph)/4;
	tcph.syn          = 1;
	tcph.window       = htons(0x8000);
	tcph.check        = 0;
	tcph.urg_ptr      = 0;

	// calculate TCP checksum
	tcph.check = tcp_sum_calc((unsigned short *) &iph.saddr,
		(unsigned short *) &iph.daddr, (unsigned short *)&tcph, 20);

	int offset = 0;
	memcpy (rawbuf+offset, &iph, sizeof(iph));
	offset += sizeof(iph);
	memcpy (rawbuf+offset, &tcph, sizeof(tcph));
	offset += sizeof(tcph);

	int slen;
	if((slen = sendto(sock, rawbuf, offset, 0, (struct sockaddr *)&dest, sizeof(dest))) < 0) {
		logger::instance()->log_error("write error %d %s\n", errno, strerror(errno));
		return -1;
	}
	logger::instance()->log_debug("send %d byte data to %s:%d\n", slen, inet_ntoa(dest.sin_addr), dstport);
//	PrintPayload((const u_char *) rawbuf, slen);
	
	return slen;
}

