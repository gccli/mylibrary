#include "dac.h"

int dac_mgmt()
{
    //////////////////////////////////////////////////////////
    // start console
    struct addrinfo hints, *serv6 = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET6;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    int err = getaddrinfo(NULL, console_port, &hints, &serv6);
    if (err != 0)
    {
	dacerror("getaddrinfo error: %d (%s)", err, gai_strerror(err));
	return 1;
    }

    int sock = socket(serv6->ai_family, serv6->ai_socktype, serv6->ai_protocol);
    int enable = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) 
    {
	dacerror("setsockopt(SO_REUSEADDR) error: %d (%s)", errno, strerror(errno));
	return 1;
    }
    if (bind(sock, serv6->ai_addr, serv6->ai_addrlen) < 0)
    {
	dacerror("failed to bind ipv6 address: %s", strerror(errno));
	close(sock);
	return 1;
    }
    listen(sock, 5);

    debugs("console started, listen in port %s", console_port);
    
    struct sockaddr_storage ss;
    char cmd[128];
    char szInfo[4096];
    while(1)
    {
	memset(&ss, 0, sizeof(ss));
	socklen_t addrlen = sizeof(ss);
	
	int clientfd = accept(sock, (struct sockaddr *) &ss, &addrlen);
	if (clientfd < 0) {
	    dacerror("accept error %s", strerror(errno));
	    sleep(1);
	    continue;
	}
	char host[96] = {0};
	char serv[32] = {0};
	int flag = 0;	
	flag = NI_NUMERICHOST | NI_NUMERICSERV; // tcpdump -nn
	if (getnameinfo((struct sockaddr *) &ss, addrlen, host, sizeof(host), serv, sizeof(serv), flag) != 0)
	    dacerror("getpeername error %s", strerror(errno));
	struct sockaddr_in6 *a6 = (struct sockaddr_in6 *) &ss;
	debugs("Receive connection %s client %s/%s", IN6_IS_ADDR_V4MAPPED(&a6->sin6_addr)?"IPv4":"IPv6", host, serv);
	
	sprintf(szInfo, "================================================================================\n"
		"	d[n]:       set debug flag to n, default reset debug flags\n"
		"	l:          list app session\n"
		"	x:          close connection\n"
		"================================================================================\n");
	send(clientfd, szInfo, strlen(szInfo), 0);
	bool iexit = false;
	while(!iexit)
	{
	    memset(cmd, 0, sizeof(cmd));
	    size_t rlen = recv(clientfd, cmd, sizeof(cmd), 0);
	    cmd[rlen] = 0;
	    switch(cmd[0])
	    {
		case 'd':
		    send(clientfd, "OK\r\n", 4, 0);
		    break;
		case 'l':
		    send(clientfd, "OK\r\n", 4, 0);
		    break;
		case 'x':
		    iexit = true;
		    break;
		default:
		    send(clientfd, szInfo, strlen(szInfo), 0);
		    break;
	    }
	}
	close(clientfd);
    }
}
