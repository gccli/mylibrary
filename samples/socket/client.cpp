#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <netdb.h>

#include "utilsock.h"

#define SNDBUF_LENGTH   4*1024
#define RCVBUF_LENGTH   16*1024

extern void *thread_receive(void *param);
extern void *thread_send(void *param);

void signal_func(int signo)
{
	printf("catch a signal %d\n", signo);
	exit(0);
}

int create_client(const char *hostname, int port, thread_func func)
{
	struct hostent *host = gethostbyname(hostname);
	if (host == NULL) {
		printf("failed to resolve domain name:%s, error:%s\n", hostname, hstrerror(h_errno));
		return 1;
	}

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr;
	socklen_t          addrlen = sizeof(addr);
	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	addr.sin_addr.s_addr = *((unsigned long* )host->h_addr);

	setsndbuflen(sock, SNDBUF_LENGTH);
	setrcvbuflen(sock, RCVBUF_LENGTH);
	if(connect(sock, (struct sockaddr *)&addr, addrlen) < 0) {
		fprintf(stderr, "connect %s\n", strerror(errno));
		return 1;
	}

	pthread_t		th;
	pthread_attr_t	thattr;
	pthread_attr_init(&thattr);
	pthread_create(&th, &thattr, func, &sock);
	pthread_join(th, NULL);

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc < 3) {
		printf("usage: %s hostname port [recv]\n", argv[0]);
		return 1;
	}
	thread_func func = thread_send;
	int port = atoi(argv[2]);
	if (argc > 3 && strcmp(argv[3], "recv") == 0) {
		func = thread_receive;
	}
	signal(SIGINT, signal_func);

	create_client(argv[1], port, func);

	return 0;
}

int main2(int argc, char *argv[])
{
	const char *hostname = argv[1];
	int port = atoi(argv[2]);
	struct hostent *host = gethostbyname(hostname);
	if (host == NULL) {
		printf("failed to resolve domain name:%s, error:%s\n", hostname, hstrerror(h_errno));
		return 1;
	}

	int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in addr;
	socklen_t          addrlen = sizeof(addr);
	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	addr.sin_addr.s_addr = *((unsigned long* )host->h_addr);

	if(connect(sock, (struct sockaddr *)&addr, addrlen) < 0) {
		fprintf(stderr, "connect %s\n", strerror(errno));
		return 1;
	}
	setrcvtimeout(sock, 10);

	printf("connect \"%s\" seccess, read data...\n", hostname);
	char buffer[1024];
	int rlen = recv(sock, buffer, sizeof(buffer), 0);
	if (rlen <= 0)
	{
		printf("read error %s\n", strerror(errno));
	}
	else {
		buffer[rlen] = 0;
		printf("response: %s\n", buffer);
	}

	return 0;
}

