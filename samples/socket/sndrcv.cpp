#include <sys/time.h>

#include "utilsock.h"
#include "utiltime.h"

static const int sread_length = 1024;

void *thread_receive(void *param)
{
	int sock = *(int *) param;

	double sec = 0.0, totallen = 0.0;
	struct timeval tv_start, tv;
	gettimeofday(&tv_start, NULL);

	char buffer[4*1024*1024];
	int  buflen, total = 0;

	for(;;) {
		buflen = readn(sock, buffer, sread_length);
		if (buflen <= 0)
			break;
		total += buflen;

		usleep(1000); //  

		totallen = 1.0*total/KBytes;
		gettimeofday(&tv, NULL);
		sec = 1.0*(tv.tv_sec-tv_start.tv_sec) + 1.0*(tv.tv_usec-tv_start.tv_usec)/1000000;
		printf("receive %8d bytes,    total %8.02f MB received, %8.02f KB/s    elapsed %4.2f second\n", buflen, totallen/KBytes, totallen/sec, sec);
	  }

	shutdown(sock, 2);
	close(sock);

	return NULL;
}

void *thread_send(void *param)
{
	int sock = *(int *) param;

	double sec = 0.0, totallen;
	struct timeval tv_start, tv;
	gettimeofday(&tv_start, NULL);

	char buffer[5*1024*1024];
	int  buflen, total = 0, sendlen[] = {1024, 32*1024, 100*1024, 200*1024, 500*1024, 1024*1024, 2*1024*1024, 5*1024*1024};

	char errmsg[1204] = "\0";

	for(int i=0; ;i++) {
		//buflen = nonblockwrite(sock, buffer, sendlen[i%8]);
		//buflen = nonblockwrite(sock, buffer, sendlen[7]);
		buflen = nonblockwrite_ext(sock, buffer, sendlen[7], errmsg);
		//if (buflen == sendlen[i%8])
		if (buflen == sendlen[7])
			total += buflen;
		else {
			fprintf(stderr, "send return %d, error:%s\n", buflen, errmsg);
			break;
		}

		totallen = 1.0*total/KBytes;
		gettimeofday(&tv, NULL);
		sec = 1.0*(tv.tv_sec-tv_start.tv_sec) + 1.0*(tv.tv_usec-tv_start.tv_usec)/1000000;
		printf("send %8d bytes,    total %8.02f MB sent, %8.02f KB/s    elapsed %4.2f second\n", buflen, totallen/KBytes, totallen/sec, sec);
	}

	shutdown(sock, 2);
	close(sock);

	return NULL;
}


