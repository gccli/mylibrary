#include "ping6.h"

static int i6sock = -1;
static struct addrinfo *i6dest;

static char *i6_addr_n(struct in6_addr *addr)
{
	static char str[64];
	inet_ntop(AF_INET6, addr, str, sizeof(str));
	return str;
}


static int i6_pr_icmph(uint8_t type, uint8_t code, uint32_t info)
{
	switch(type) {
	case ICMP6_DST_UNREACH:
		printf("Destination unreachable: ");
		switch (code) {
		case ICMP6_DST_UNREACH_NOROUTE:
			printf("No route");
			break;
		case ICMP6_DST_UNREACH_ADMIN:
			printf("Administratively prohibited");
			break;
		case ICMP6_DST_UNREACH_BEYONDSCOPE:
			printf("Beyond scope of source address");
			break;
		case ICMP6_DST_UNREACH_ADDR:
			printf("Address unreachable");
			break;
		case ICMP6_DST_UNREACH_NOPORT:
			printf("Port unreachable");
			break;
		default:
			printf("Unknown code %d", code);
			break;
		}
		break;
	case ICMP6_PACKET_TOO_BIG:
		printf("Packet too big: mtu=%u", info);
		if (code)
			printf(", code=%d", code);
		break;
	case ICMP6_TIME_EXCEEDED:
		printf("Time exceeded: ");
		if (code == ICMP6_TIME_EXCEED_TRANSIT)
			printf("Hop limit");
		else if (code == ICMP6_TIME_EXCEED_REASSEMBLY)
			printf("Defragmentation failure");
		else
			printf("code %d", code);
		break;
	case ICMP6_PARAM_PROB:
		printf("Parameter problem: ");
		if (code == ICMP6_PARAMPROB_HEADER)
			printf("Wrong header field ");
		else if (code == ICMP6_PARAMPROB_NEXTHEADER)
			printf("Unknown header ");
		else if (code == ICMP6_PARAMPROB_OPTION)
			printf("Unknown option ");
		else
			printf("code %d ", code);
		printf ("at %u", info);
		break;
	case ICMP6_ECHO_REQUEST:
		printf("Echo request");
		break;
	case ICMP6_ECHO_REPLY:
		printf("Echo reply");
		break;
	case MLD_LISTENER_QUERY:
		printf("MLD Query");
		break;
	case MLD_LISTENER_REPORT:
		printf("MLD Report");
		break;
	case MLD_LISTENER_REDUCTION:
		printf("MLD Reduction");
		break;
	default:
		printf("unknown icmp type: %u", type);
	}

	return 0;
}

int i6_icmp_error()
{
	int res;
	char cbuf[512];
	struct iovec  iov;
	struct msghdr msg;
	struct cmsghdr *cmsg;
	struct sock_extended_err *e;
	struct icmp6_hdr icmph;
	struct sockaddr_in6 target;
	int net_errors = 0;
	int local_errors = 0;
	int saved_errno = errno;

	iov.iov_base = &icmph;
	iov.iov_len = sizeof(icmph);
	msg.msg_name = (void*)&target;
	msg.msg_namelen = sizeof(target);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;
	msg.msg_control = cbuf;
	msg.msg_controllen = sizeof(cbuf);

	res = recvmsg(i6sock, &msg, MSG_ERRQUEUE|MSG_DONTWAIT);
	if (res < 0)
		goto out;

	e = NULL;
	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_IPV6) {
			if (cmsg->cmsg_type == IPV6_RECVERR)
				e = (struct sock_extended_err *)CMSG_DATA(cmsg);
		}
	}
	if (e == NULL)
		abort();

	if (e->ee_origin == SO_EE_ORIGIN_LOCAL) {
		local_errors++;
		fprintf(stderr, "ping: local error: %s\n", strerror(e->ee_errno));
	} else if (e->ee_origin == SO_EE_ORIGIN_ICMP6) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)(e+1);

		if ((size_t)res < sizeof(icmph) ||
		    memcmp(&target.sin6_addr, &((struct sockaddr_in6 *) i6dest->ai_addr)->sin6_addr, 16) ||
		    icmph.icmp6_type != ICMP6_ECHO_REQUEST ||
		    icmph.icmp6_id != getpid()) {
			/* Not our error, not an error at all. Clear. */
			saved_errno = 0;
			goto out;
		}

		net_errors++;

		printf("From %s icmp_seq=%u ", i6_addr_n(&sin6->sin6_addr), ntohs(icmph.icmp6_seq));
		i6_pr_icmph(e->ee_type, e->ee_code, e->ee_info);
		putchar('\n');
		fflush(stdout);
	}

out:
	errno = saved_errno;
	return net_errors ? : -local_errors;
}

int i6_icmp_reply(struct msghdr *msg, int cc, void *addr, struct timeval *tv)
{
	struct sockaddr_in6 *from = (struct sockaddr_in6 *) addr;
	uint8_t *buf = (uint8_t *) msg->msg_iov->iov_base;
	struct cmsghdr *c;
	struct icmp6_hdr *icmph;
	int hops = -1;

	for (c = CMSG_FIRSTHDR(msg); c; c = CMSG_NXTHDR(msg, c)) {
		if (c->cmsg_level != SOL_IPV6)
			continue;
		switch(c->cmsg_type) {
		case IPV6_HOPLIMIT:
			if (c->cmsg_len < CMSG_LEN(sizeof(int)))
				continue;
			memcpy(&hops, CMSG_DATA(c), sizeof(hops));
			break;
		default:
			break;
		}
	}

	/* Now the ICMP part */
	icmph = (struct icmp6_hdr *) buf;
	if (cc < 8) {
		return 1;
	}

	uint8_t *ptr = (uint8_t *) icmph + sizeof(*icmph);

	if (icmph->icmp6_type == ICMP6_ECHO_REPLY) {
		if (icmph->icmp6_id != getpid())
			return 1;
		long triptime = 0;
		if ((size_t)cc >= 8+sizeof(struct timeval)) {
			struct timeval tmp_tv;
			memcpy(&tmp_tv, ptr, sizeof(tmp_tv));

			if ((tv->tv_usec -= tmp_tv.tv_usec) < 0) {
				--tv->tv_sec;
				tv->tv_usec += 1000000;
			}
			tv->tv_sec -= tmp_tv.tv_sec;
			triptime = tv->tv_sec * 1000000 + tv->tv_usec;
		}


		printf("%d bytes from %s:", cc, i6_addr_n(&from->sin6_addr));
		
		struct icmphdr *icp = (struct icmphdr *)icmph;
		printf(" icmp_seq=%u", ntohs(icp->un.echo.sequence));
		if (hops >= 0)
			printf(" ttl=%d", hops);
	
		if (triptime >= 100000)
			printf(" time=%ld ms", triptime/1000);
		else if (triptime >= 10000)
			printf(" time=%ld.%01ld ms", triptime/1000,
				   (triptime%1000)/100);
		else if (triptime >= 1000)
			printf(" time=%ld.%02ld ms", triptime/1000,
				   (triptime%1000)/10);
		else
			printf(" time=%ld.%03ld ms", triptime/1000,
				   triptime%1000);

	} else {
		int nexthdr;
		struct ip6_hdr *iph1 = (struct ip6_hdr*)(icmph+1);
		struct icmp6_hdr *icmph1 = (struct icmp6_hdr *)(iph1+1);

		/* We must not ever fall here. All the messages but
		 * echo reply are blocked by filter and error are
		 * received with IPV6_RECVERR. Ugly code is preserved
		 * however, just to remember what crap we avoided
		 * using RECVRERR. :-)
		 */

		if ((size_t)cc < 8+sizeof(struct ip6_hdr)+8)
			return 1;

		//if (memcmp(&iph1->ip6_dst, &whereto.sin6_addr, 16))
		//	return 1;

		nexthdr = iph1->ip6_nxt;

		if (nexthdr == 44) {
			nexthdr = *(uint8_t*)icmph1;
			icmph1++;
		}
		if (nexthdr == IPPROTO_ICMPV6) {
			if (icmph1->icmp6_type != ICMP6_ECHO_REQUEST ||
			    icmph1->icmp6_id != getpid())
				return 1;
			/*
			acknowledge(ntohs(icmph1->icmp6_seq));
			if (working_recverr)
				return 0;
			nerrors++;
			if (options & F_FLOOD) {
				write_stdout("\bE", 2);
				return 0;
			}*/
			//print_timestamp();
			printf("From %s: icmp_seq=%u ", i6_addr_n(&from->sin6_addr), ntohs(icmph1->icmp6_seq));
		} else {
			/* We've got something other than an ECHOREPLY */
			/*if (!(options & F_VERBOSE) || uid)
				return 1;
			print_timestamp();*/
			printf("From %s: ", i6_addr_n(&from->sin6_addr));
		}
		i6_pr_icmph(icmph->icmp6_type, icmph->icmp6_code, ntohl(icmph->icmp6_mtu));
	}

	printf("\n");
	return 0;
}

static int i6_echo(unsigned char *outpack, int seq)
{
	int i=0, len=0;
	struct icmp6_hdr *icmph = (struct icmp6_hdr *)outpack;
	icmph->icmp6_type = ICMP6_ECHO_REQUEST;
	icmph->icmp6_code = 0;
	icmph->icmp6_cksum = 0;
	icmph->icmp6_seq = htons(seq+1);
	icmph->icmp6_id = getpid();

	gettimeofday((struct timeval *)&outpack[sizeof(struct icmp6_hdr)], NULL);

	int datalen = 512-sizeof(struct icmp6_hdr)-sizeof(struct timeval);

	if (config.debug) {
		printf("Build icmp echo request, header length %zu timeval len %zu data len %d\n",
			sizeof(struct icmp6_hdr), sizeof(struct timeval), datalen);
	}

	for(i=sizeof(struct icmp6_hdr)+sizeof(struct timeval); i < datalen; ++i)
		outpack[i] = 'x';
	len = i;

	return len;
}

int i6ping(const char *host)
{
	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET6;
	hints.ai_flags = AI_ALL | AI_CANONNAME;
	hints.ai_socktype = SOCK_RAW;
	int err = getaddrinfo(host, NULL, &hints, &i6dest);
	if (err != 0 || i6dest == NULL)
	{
		fprintf (stderr, "getaddrinfo error: %d (%s)\n", err, gai_strerror(err));
		return NULL;
	}

	if (!(i6dest->ai_family == AF_INET6 && i6dest->ai_socktype == SOCK_RAW)) {
		fprintf (stderr, "not specific address\n");
		return 1;
	}

	i6sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

	int on = 1;

	if (setsockopt(i6sock, SOL_IPV6, IPV6_RECVERR, (char *)&on, sizeof(on))) {
		fprintf(stderr, "setsockopt(IPV6_RECVERR)\n");
	}

	if (setsockopt(i6sock, IPPROTO_IPV6, IPV6_RECVPKTINFO,  &on, sizeof(on))) {
		fprintf(stderr, "setsockopt(IPV6_RECVPKTINFO)\n");
	}

	if (setsockopt(i6sock, SOL_SOCKET, SO_TIMESTAMP, &on, sizeof(on))) {
		fprintf(stderr, "setsockopt(SO_TIMESTAMP)\n");
	}

	on = 2;
	if (setsockopt(i6sock, SOL_RAW, IPV6_CHECKSUM, &on, sizeof(on)) < 0) {
		fprintf(stderr, "setsockopt(RAW_CHECKSUM) failed - try to continue.");
	}

	if (config.ttl > 0) {
		on = config.ttl;
		setsockopt(i6sock, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &on, sizeof(on));
		setsockopt(i6sock, IPPROTO_IPV6, IPV6_UNICAST_HOPS,   &on, sizeof(on));
	}

	/* Set timeout */
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(i6sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));

	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(i6sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));


	struct icmp6_filter filter;
	ICMP6_FILTER_SETBLOCKALL(&filter);
	ICMP6_FILTER_SETPASS(ICMP6_ECHO_REPLY, &filter);

	if (setsockopt(i6sock, IPPROTO_ICMPV6, ICMP6_FILTER, &filter,sizeof(struct icmp6_filter)) < 0) {
		fprintf(stderr, "setsockopt(ICMP6_FILTER) failed - try to continue.");
	}
	if (setsockopt(i6sock, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof(on)) < 0) {
		fprintf(stderr, "setsockopt(IPV6_RECVHOPLIMIT) failed - try to continue.");
	}

	unsigned char outpack[1024];
	unsigned char cmsgbuf[4096];
	int cmsglen = 0;

	if (config.ifname[0]) {
		struct cmsghdr *cmsg;
		struct in6_pktinfo *ipi;

		cmsg = (struct cmsghdr*)(cmsgbuf+cmsglen);
		cmsglen += CMSG_SPACE(sizeof(*ipi));
		cmsg->cmsg_len = CMSG_LEN(sizeof(*ipi));
		cmsg->cmsg_level = SOL_IPV6;
		cmsg->cmsg_type = IPV6_PKTINFO;

		ipi = (struct in6_pktinfo*)CMSG_DATA(cmsg);
		memset(ipi, 0, sizeof(*ipi));
		ipi->ipi6_ifindex = if_nametoindex(config.ifname);
	}

	int i, len, cc, ntransmitted=0;
	len = i6_echo(outpack, ntransmitted);

	printf("PING %s(%s) ", config.host, i6_addr_n(&((struct sockaddr_in6 *) i6dest->ai_addr)->sin6_addr));
	if (config.ifname[0]) {
		printf("from %s %s: ", config.ifname, "");
	}
	printf("%d data bytes\n", len);


	if (config.count < 4) config.count = 4;
	for(i=0; i<config.count; ++i)
	{
		struct msghdr mhdr;
		struct iovec iov;
		iov.iov_len  = len;
		iov.iov_base = outpack;

		memset(&mhdr, 0, sizeof(mhdr));
		mhdr.msg_name = i6dest->ai_addr;
		mhdr.msg_namelen = sizeof(struct sockaddr_in6);
		mhdr.msg_iov = &iov;
		mhdr.msg_iovlen = 1;
		mhdr.msg_control = cmsgbuf;
		mhdr.msg_controllen = cmsglen;
		
		cc = sendmsg(i6sock, &mhdr, 0);
		if (cc < 0) {
			fprintf(stderr, "sendmsg: %s\n", strerror(errno));
			return -1;
		}

		len = i6_echo(outpack, ++ntransmitted);

		for (;;) {
			unsigned char recvbuf[1024] = {0};
			unsigned char addrbuf[128];

			struct timeval *recv_timep = NULL;
			int not_ours = 0; /* Raw socket can receive messages destined to other running pings. */
			iov.iov_len = sizeof(recvbuf);
			iov.iov_base = recvbuf;
			memset(&mhdr, 0, sizeof(mhdr));

			mhdr.msg_name = addrbuf;
			mhdr.msg_namelen = sizeof(addrbuf);
			mhdr.msg_iov = &iov;
			mhdr.msg_iovlen = 1;
			mhdr.msg_control = cmsgbuf;
			mhdr.msg_controllen = sizeof(cmsgbuf);
			cc = recvmsg(i6sock, &mhdr, 0);

			if (cc < 0) {
				if (errno == EAGAIN || errno == EINTR)
					break;
				else { i6_icmp_error(); }
			} else {
				struct cmsghdr *c;
				for (c = CMSG_FIRSTHDR(&mhdr); c; c = CMSG_NXTHDR(&mhdr, c)) {
					if (c->cmsg_level != SOL_SOCKET ||
						c->cmsg_type != SO_TIMESTAMP)
						continue;
					if (c->cmsg_len < CMSG_LEN(sizeof(struct timeval)))
						continue;
					recv_timep = (struct timeval*)CMSG_DATA(c);
				}
				not_ours = i6_icmp_reply(&mhdr, cc, addrbuf, recv_timep);
				if (!not_ours)
					break;
			}
		}
	}

	close(i6sock);

	return 0;
}



