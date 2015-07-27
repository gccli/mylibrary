#include <pthread.h>
#include <signal.h>
#include "utilsock.h"

#define SNDBUF_LENGTH   8*1024
#define RCVBUF_LENGTH   16*1024

extern void *thread_receive(void *param);
extern void *thread_send(void *param);

void signal_func(int signo)
{
	printf("catch a signal %d\n", signo);
	exit(0);
}

int main(int argc, char* argv[])
{	
	int port = 23;
	thread_func func = thread_receive;
	if (argc > 1) {
		port = atoi(argv[1]);
	}
	if (argc > 2 && strcmp(argv[2], "send") == 0) {
		func = thread_send;			
	}

	signal(SIGINT, signal_func);

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr, remote;
	socklen_t          addrlen = sizeof(addr);
	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	setreuse(sock);
	if(bind(sock, (struct sockaddr *)&addr, addrlen) < 0) {
		fprintf(stderr, "bind %s\n", strerror(errno));
		return 1;
	}

	setrcvbuflen(sock, RCVBUF_LENGTH);
	setsndbuflen(sock, SNDBUF_LENGTH);
	if(listen(sock, 5) < 0) {
		fprintf(stderr, "listen %s\n", strerror(errno));
		return 1;
	}
	
	int clisock = -1;
	while (1) {
		memset(&remote, 0, addrlen);
		if ((clisock = accept(sock, (struct sockaddr *)&remote, &addrlen)) < 0) {
			fprintf(stderr, "accept %s\n", strerror(errno));
			return 1;
		}

		printf("accept client address %s:%d socket:%d MSS:%d\n", inet_ntoa(remote.sin_addr), ntohs(remote.sin_port), clisock, getMSS(clisock));
		pthread_t		th;
		pthread_attr_t	thattr;
		pthread_attr_init(&thattr);
		pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
		pthread_create(&th, &thattr, func, &clisock);

	}

	close(sock);

	return 0;
}
