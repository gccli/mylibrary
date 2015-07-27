#include <stdio.h>
#include <signal.h>
#include <sys/syscall.h>

#include "nwt_socket.h"
#include "unixsocket.h"
#include "epollsvr.h"
#include "threadpool.h"
#include "nwt_config.h"
#include "logger.h"

extern int    simple_udpserver(void* param);
extern int    simple_udpclient(void* param);
extern int    simple_udpclient_stdin(void* param);
extern int    simple_tcpclient(void* param);
extern int    simple_tcpserver(void* param);
extern void   RawUDPSocketSend(const char* srciplist, int srcport,  const char* dstip, int dstport);
extern void   RawTCPSocketConnect(const char* srciplist, int srcport, const char* dstip, int dstport);

extern void   signal_func(int signo);
extern void   parse_command_line(int argc, char* argv[]);

/**
 * nwtutil - Network Testing Tools
 */

int main(int argc, char* argv[])
{
	initialize_default_conf();
	parse_command_line(argc, argv);
	logger::instance()->init(NULL, true);

	signal(SIGINT, signal_func);
	signal(SIGCHLD, signal_func);

	
	UDBClient* uclient = NULL;
	UDBServer* userver = NULL;
	TCPClient* tclient = NULL;
	TCPServer* tserver = NULL;
	
	/******** TCP CLIENT MODE ********/
	if (gl_listen_mode == TCPCLIENT) 
	{
		printf ("Listen Mode: TCP Client\n");
		tclient = new TCPClient;
		//tclient->createlink(gl_destination_ipaddress, gl_destination_port, simple_tcpclient);
		if (tclient->createlink(gl_destination_ipaddress, gl_destination_port, 3) == 0) {
			simple_tcpclient(tclient);
		}
	}
	/******** TCP SERVER MODE ********/
	else if ((gl_listen_mode & ~TCPSERVER) == 0) 
	{
		printf ("Listen Mode: TCP Server\n");
		tserver = new TCPServer;
		tserver->init(gl_source_port);
		tserver->start(simple_tcpserver);
	}
	else if ((gl_listen_mode & ~(TCPSERVER|MULTIPLEX)) == 0)
	{
		printf ("Listen Mode: TCP Multiplex Server\n");
		tserver = new Epollsvr;			
		tserver->init(gl_source_port);
		tserver->start();
	}
	else if ((gl_listen_mode & ~(TCPSERVER|THREADPOOL)) == 0)
	{
		printf ("Listen Mode: TCP Server with Thread Pool\n");
		tserver = new ThreadPoolSvr(gl_thread_pool_size);
		tserver->init(gl_source_port);
		tserver->start();
	}
	/******** UDP CLIENT MODE ********/
	else if ((gl_listen_mode & ~UDPCLIENT) == 0)
	{
		printf ("Listen Mode: UDP Client\n");
		uclient = new UDBClient;
		if (gl_ipmulti_outputif != NULL)
			uclient->setOutputIF(gl_ipmulti_outputif);
		if (gl_sendfilename != NULL) {
			uclient->sendfile(gl_sendfilename, gl_send_count);
		}
		else {
			if (gl_sendbuffer != NULL)
				uclient->createlink(gl_destination_ipaddress, gl_destination_port, simple_udpclient);
			else
				uclient->createlink(gl_destination_ipaddress, gl_destination_port, simple_udpclient_stdin);
		}
	}
	/******** UDP SERVER MODE ********/
	else if ((gl_listen_mode & ~UDPSERVER) == 0)                   
	{
		printf ("Listen Mode: UDP Unicast Server\n");
		userver = new UDBServer;
		userver->start(gl_source_port, UNICAST_MODE, simple_udpserver);
	}
	/******** UDP Multicast SERVER MODE ********/
	else if ((gl_listen_mode & ~(UDPSERVER|MULTICAST)) == 0)
	{
		printf ("Listen Mode: UDP Multicast Server\n");
		userver = new UDBServer(gl_ipmulti_groupaddr);
		if (gl_ipmulti_inputif != NULL)
			userver->setInputIF(gl_ipmulti_inputif);
		userver->start(gl_source_port, MULTICAST_MODE, simple_udpserver);
	}
	/******** RAW TCP Socket MODE ********/
	else if ((gl_listen_mode & ~RAW_IP_DATAGRAM) == 0)
	{
		printf ("Listen Mode: TCP Raw Socket\n");
		if (gl_source_ipaddresses == NULL)
		{
			logger::instance()->log_error("Source address list is empty, please specify with '--saddrlist' option.\n");
			return 1;
		}		
		RawTCPSocketConnect(gl_source_ipaddresses, gl_source_port, "172.16.0.10", 80);
	}
	/******** RAW UDP Socket MODE ********/
	else if ((gl_listen_mode & ~(UDPCLIENT|RAW_IP_DATAGRAM)) == 0)
	{
		printf ("Listen Mode: UDP Raw Socket\n");
		if (gl_source_ipaddresses == NULL)
		{
			logger::instance()->log_error("Source address list is empty, please specify with '--saddrlist' option.\n");
			return 1;
		}
		RawUDPSocketSend(gl_source_ipaddresses, gl_source_port, "172.16.0.10", 8080);
	}

	if (uclient) delete uclient;
	if (userver) delete userver;
	if (tclient) delete tclient;
	if (tserver) delete tserver;

	return 0;
}

