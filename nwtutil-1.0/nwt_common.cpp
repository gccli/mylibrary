#include "nwt_socket.h"
#include "nwt_config.h"
#include <arpa/inet.h>

#include <pthread.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <termios.h>

#include <vector>
#include <string>
using namespace std;

#include "utiltime.h"
#include "logger.h"

int simple_tcpclient(void* param)
{
	TCPClient* client = (TCPClient* ) param;

	// timing
	double tstart = usecTStart();

	int read_len;
	char *line = NULL;
	size_t  rlen;
	printf ("> ");
	while ((read_len = getline(&line, &rlen, stdin)) != EOF) {
		int slen = client->writedata(line, read_len);
		printf("send length %d\n", slen);
		printf ("> ");
		if (gl_spec_sleep)
			usleep (gl_sleep_usec);
	}

	double timecost = usecTEnd(tstart);
	fprintf (stderr, "\n++++++++++++++++++++++++++++++++++++++++\n");
	fprintf (stderr, "total time cost: %lf\n", timecost);

	if (line)
		free(line);

  return 0;
}

static void* thread_io(void* param)
{
	int clientfd = *(int* ) param;
	int lwpid = gettid();
	printf ("io_thread<%d> created, process sockfd: %d.\n", lwpid, clientfd);

	char tmpbuf[64];
	sprintf (tmpbuf, "/tmp/%d.out", lwpid);
	FILE* fp = fopen (tmpbuf, "w");

	struct sockaddr_in raddr;
	memset (&raddr, 0, sizeof(raddr));
	socklen_t addrlen = sizeof(raddr);
	if (getpeername(clientfd, (struct sockaddr *) &raddr, &addrlen) == 0) 
		;
		
	unsigned char buffer[4096];
	while(true) {
		int len = recv (clientfd, buffer, sizeof(buffer), 0);
		if (len <= 0) {
			break;
		}
		fwrite (buffer, 1, len, fp);
		fflush (fp);
	}// end while

	close (clientfd);
	if (fp) 
		fclose (fp);
	
	return NULL;
}

int simple_tcpserver(void* param)
{
	pthread_t      th;
	pthread_attr_t thattr;
	pthread_attr_init (&thattr);
	pthread_attr_setdetachstate(&thattr, PTHREAD_CREATE_DETACHED);
	size_t stacksize = 1024*1024;
	pthread_attr_setstacksize(&thattr, stacksize);
	pthread_attr_getstacksize(&thattr, &stacksize);
	if (pthread_create(&th, &thattr, thread_io, param) == 0) {
		usleep (10);
		printf ("    thread create success with stack size = %ld\n", stacksize);
		return 0;
	}
	
	return 0;
}

static int process_raw_string(const char* rawstr, char* hexstr, int* length)
{
	int rawlen = strlen(rawstr);
	int i,j;

	char hexbuffer[rawlen];
	for (i=0, j=0; i<rawlen; ++i) {
		if (isdigit(rawstr[i]) || (rawstr[i] >= 'A' && rawstr[i] <= 'F') ||	(rawstr[i] >= 'a' && rawstr[i] <= 'f')) 
		{
			hexbuffer[j++] = rawstr[i];
		}
	}
	hexbuffer[j] = 0;

	int hexlen = strlen(hexbuffer);
	if (hexlen == 0 || hexlen%2 != 0) {
		return 1;
	}

	char str[3];
	for (i=0,j=0; i<hexlen; i+=2) {
		strncpy(str, hexbuffer+i, 2);
		str[2] = 0;
		hexstr[j++] = strtol(str, NULL, 16);
	}
	*length = j;

	return 0;
}

static int process_stdin(int dstsock, uint32_t dstip, uint16_t dstport)
{
	char    *line = NULL;
	size_t  rlen;

	struct sockaddr_in dstaddr;
	memset(&dstaddr, 0, sizeof(dstaddr));
	dstaddr.sin_family = AF_INET;
	dstaddr.sin_port   = dstport;
	dstaddr.sin_addr.s_addr = dstip;

	int read_len = getline(&line, &rlen, stdin);
	if (read_len > 0) {
		int slen = sendto(dstsock, line, read_len, 0, (struct sockaddr *)&dstaddr, sizeof(dstaddr));
		printf("send length %d\n", slen);
	}

	if (line)
		free(line);
	
	return read_len;
}

static int process_stdin_hex(int dstsock)
{
	const int buflen = 1024;
	int  readlen;
	char inbuffer[buflen*2];
	char outbuffer[buflen];

	if (!isatty(STDIN_FILENO)) {
		printf ("stdin not a tty\n");
		return -1;
	}

	struct termios ts, ots;
	if (tcgetattr(STDIN_FILENO, &ts)) {
		fprintf (stderr, "tcgetattr error %d %s\n", errno, strerror(errno));
		return -1;
	}
	ots = ts;
	ts.c_lflag &= ~(ICANON | ECHO);
	if (tcsetattr(STDIN_FILENO, TCSANOW, &ts) < 0) {
		fprintf (stderr, "tcsetattr error %d %s\n", errno, strerror(errno));
		return -1;
	}

	readlen = read(STDIN_FILENO, inbuffer, sizeof(inbuffer));
	inbuffer[readlen] = 0;
	printf ("\nread from stdin:\n%s\nlength: %d\n", inbuffer, readlen);
	
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &ots) < 0) {
		fprintf (stderr, "tcsetattr error %d %s\n", errno, strerror(errno));
		return -1;
	}

	int length = 0;
	process_raw_string(inbuffer, outbuffer, &length);
	return send(dstsock, outbuffer, length, 0);
}

int simple_udpclient_stdin(void* param)
{
	dstinfo *dst = (dstinfo *) param;
	int sock = dst->sock;

	struct sockaddr_in dstaddr;
	socklen_t addrlen = sizeof(dstaddr);

	const int line_length  = 1024;
	char  rdbuf[line_length];
	int read_len;

	struct timeval tv;
	fd_set  rdset;
	FD_ZERO(&rdset);
	int stdin_fileno = fileno(stdin);

	while (!gl_terminate) {
		tv.tv_sec = 1; tv.tv_usec = 0;
		FD_SET(sock, &rdset);
		FD_SET(stdin_fileno, &rdset);
		int maxfds = (sock > stdin_fileno)?(sock+1):(stdin_fileno+1);
		int nready = select (maxfds, &rdset, NULL, NULL, &tv);
		if (nready == 0) continue;
		else if (nready < 0) {
			if (errno == EINTR) continue;
			perror ("select");
			break;
		}

		if (FD_ISSET(sock, &rdset)) {
			if ((read_len = recvfrom(sock, rdbuf, sizeof(rdbuf), 0, (struct sockaddr*) &dstaddr, &addrlen)) == -1){
				fprintf (stderr, "recv() error: %d (%s)\n", errno, strerror(errno));
				break;
			}
			rdbuf[read_len] = 0;
			printf ("read from server %s:%d -- %s\n", inet_ntoa(dstaddr.sin_addr), ntohs(dstaddr.sin_port), rdbuf);
		}
		if (FD_ISSET(stdin_fileno, &rdset)) {
			if (!gl_process_hex_stdin && process_stdin(sock, dst->ip, dst->port) > 0)
				continue;
			else if (gl_process_hex_stdin && process_stdin_hex(sock) > 0)
				continue;
			else break;
		}		
	}

	return 0;
}


int simple_udpclient(void* param)
{
	int sock = *(int*) param;
	int buflen = strlen(gl_sendbuffer);

	if (gl_process_hex_stdin) {
		char sendbuffer[1024];
		int  length;
		process_raw_string(gl_sendbuffer, sendbuffer, &length);
		return send(sock, sendbuffer, length, 0);
	}
	
	return send(sock, gl_sendbuffer, buflen, 0);
}


int simple_udpserver(void* param)
{
	int  sock = *(int* )param;
	int  flags = 0;

	char rdbuf[1024];
	int  rlen;
	
	struct sockaddr_in raddr;
	socklen_t addrlen = sizeof(raddr);
	memset (&raddr, 0, addrlen);

	struct timeval tv;
	fd_set fdset;
	FD_ZERO(&fdset);

	while (!gl_terminate) {
		tv.tv_sec = 1; tv.tv_usec = 0;
		FD_SET (sock, &fdset);

		int count = select (sock+1, &fdset, NULL, NULL, &tv);
		if (count == 0) continue;
		else if (count < 0) {
			if (errno == EINTR) continue;
			perror ("select");
			break;
		}

		if (FD_ISSET(sock, &fdset)) {
			rlen = recvfrom (sock, rdbuf, sizeof(rdbuf), flags, (struct sockaddr*) &raddr, &addrlen);
			if (rlen == 0) continue;
			if (rlen < 0) {
				fprintf (stderr, "recvfrom() error: %d (%s)\n", errno, strerror(errno));
				break;
			}
			char straddr[INET_ADDRSTRLEN];
			inet_ntop (AF_INET, &raddr.sin_addr, straddr, sizeof(straddr));
			char response[64] = "\0";
			sprintf (response, "%s:%d", straddr, ntohs(raddr.sin_port));
			logger::instance()->log_info("Receive from %s, length = %d\n", response, rlen);
			PrintPayload((u_char*)rdbuf, rlen);
			
			sendto (sock, response, strlen(response), 0, (struct sockaddr*) &raddr, addrlen);
		}

	}

	return 0;
}

static char* trimspace(char* buffer)
{
	char* left = buffer;
	for (; (*left == ' ' || *left == '\r' || *left == '\n'|| *left == '\t'); ++left) // trim left
		;
	
	int   buflen = strlen(buffer);
	char* right  = buffer + buflen - 1;
	for (; (*right == ' ' || *right == '\r' || *right == '\n'|| *right == '\t'); right--)
		;

	right++;
	*right = 0;

	return left;	
}

static int split(const char* reglist, const char* delim, std::vector<string>& v)
{
	int   bufflen = strlen(reglist) + 1;
	char* buffer  = new char[bufflen];
	strcpy (buffer, reglist);

	char* p = strtok(buffer, delim);
	while (p != NULL) {
		string sip = trimspace(p);
		v.push_back(sip);
		p = strtok(NULL, delim);
	}
	
	return 0;
}

static int split_iplist(const char* iplist, std::vector<string>& v)
{
	char* p = NULL;
	p = (char *)strchr(iplist, ',');
	if (p != NULL)
		return split(iplist, ",", v);

	p = (char *)strchr(iplist, ';');
	if (p != NULL)
		return split(iplist, ";", v);

	p = (char *)strchr(iplist, '-');
	if (p == NULL) {
		return 1;
	}
	char ip_range[1024] = "\0";
	strcpy (ip_range, iplist);

	p = strpbrk (ip_range, "-");
	*p = 0;	p++;
	unsigned int nstart, nend;
	
	// transfer network byte order to host byte order
	nstart = ntohl (inet_addr( trimspace(ip_range) )); 
	nend   = ntohl (inet_addr( trimspace(p) ));
	int diff = nend - nstart;
	if (nstart>nend || diff > 0xffff) { 
		logger::instance()->log_error("Bad IP address range [%d-%d] (host byte order)\n", nstart, nend);
		logger::instance()->log_error("IP address range difference is %d\n", diff);
		return 1;
	}
	struct sockaddr_in saddr;
	memset (&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	
	for (unsigned int ip = nstart; ip <= nend; ++ip) {
		saddr.sin_addr.s_addr = htonl(ip);
		string sip = inet_ntoa(saddr.sin_addr);
		v.push_back(sip);
	}

	return 0;
}

void RawUDPSocketSend(const char* srciplist, int srcport, const char* dstip, int dstport)
{
	vector<string> v_srcip;
	if (split_iplist(srciplist, v_srcip) != 0) {
		logger::instance()->log_error("Split IP address list failure!\n");
		return ;
	}
	int i, len;
	char buffer[256];
	RAWSocket* usock = NULL;
	
	vector<string>::iterator it = v_srcip.begin();
	for (i=0; it != v_srcip.end(); ++it,i++) {
		sprintf (buffer, "%d", i);
		len = strlen(buffer);
		usock = new RAWSocket(IPPROTO_UDP);
		usock->buildRawUDPData(it->c_str(), srcport, dstip, dstport, buffer, len);
		delete usock;
	}
}

void RawTCPSocketConnect(const char* srciplist, int srcport, const char* dstip, int dstport)
{
	vector<string> v_srcip;
	if (split_iplist(srciplist, v_srcip) != 0) {
		logger::instance()->log_error("Split IP address list failure!\n");
		return ;
	}
	int i;
	RAWSocket* tsock = NULL;
	vector<string>::iterator it = v_srcip.begin();
	for (i=0; it != v_srcip.end(); ++it,i++) {
		tsock = new RAWSocket(IPPROTO_TCP);
		tsock->buildTcpSYN(it->c_str(), srcport, dstip, dstport);
		delete tsock;
	}	
}


