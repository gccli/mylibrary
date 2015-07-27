/*
 * main auxiliary
 * 
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <signal.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "nwt_config.h"

void signal_func(int signo)
{
	printf ("receive signal %d\n", signo);

	if (signo == SIGINT) {
		gl_terminate = true;
		exit (0);
	}
	else if (signo == SIGCHLD) {
		int status;
		pid_t child = waitpid (-1, &status, 0);
		if (WIFEXITED(status)) {
			printf ("child process<%d> terminated normally, return code %d\n", child, WEXITSTATUS(status));
		}
		else if (WIFSIGNALED(status)) {
			printf ("child process<%d> was terminated by a signal %d\n", child, WTERMSIG(status));
		}
	}
	
}


static void usage()
{
	printf ("NAME\n");
	printf ("    nwtutil - Network Test Tools.\n\n");
	printf ("SYNOPSIS\n");
	printf ("    nwtutil [ -hvuD ] [ -c count ] [ -f file ] [ -o outputif ] [ -r rate ]\n");
	printf ("            [ --saddr ADDR ] [ --sport PORT ] [ --ttl TTL ] [ -S SLEEPTIME ]\n");	
	printf ("            desthost:destport\n\n");

	printf ("    nwtutil [ -hvuD ] [ -L[udp | multicast ] [ -M ] [ -i inputif ] [-G GROUPADDR]\n");
	printf ("            [ -S SLEEPTIME ] [ --threadpool[=NUMBER] ]\n\n");
  
  /*
  printf ("\nOptions\n");
  printf ("  -c, --count=n\n");
  printf ("        specify broadcast count, default is 1, if loop is set, this option is ignored.\n");
  printf ("  -h, --help\n");
  printf ("        show this help.\n");
  printf ("  -l, --loop\n");
  printf ("        loop mode, when boardcast a file.\n");
  printf ("  -f, --file=filename\n");
  printf ("        specify a file for boardcast.\n");
  printf ("  -o, --outputif=oif\n");
  printf ("        specify output interface, e.g. eth1, when destination is ip multicast.\n");
  printf ("  -i, --inputif=iif\n");
  printf ("        specify input interface, e.g. eth1, when destination is ip multicast.\n");
  printf ("  -r, --rate=BYTES\n");
  printf ("        specify broadcast rate with BYTES, BYTES can be followed by the following suffixes.\n");
  printf ("        K means 1024 Byte, M means 1024*1024 Byte, case-insensitive. e.g. 3.5M.\n");
  printf ("  -v\n");
  printf ("        verbose output, contain a process bar.\n");
  printf ("  --daemon\n");
  printf ("        start playout as a daemon process.\n");
  printf ("  --saddr=addr\n");
  printf ("        specify source ip address with addr, dotted decimal notation.\n");
  printf ("  --sport=n\n");
  printf ("        specify source port with n.\n");
  printf ("  --ttl=n\n");
  printf ("        specify ttl for ip datagram ttl with n.\n");
  printf ("  --syslog=facility\n");
  printf ("        specify log facility, where facility between 0 and 7, default is 0.\n");
  printf ("        0 stand for LOG_LOCAL0, see syslog(3).\n");

  printf ("\nExamples\n");
  printf ("  ./playout -v -c2 -f loader.mpg 10.10.168.60:10002\n");
  printf ("        send file loader.mpg to destination 10.10.168.60:10002, count is 2, verbose mode.\n");
  printf ("  ./playout --daemon --loop -o eth1 -f loader.mpg 225.10.10.10:10002\n");
  printf ("        daemon mode, destination is ip multicast, use eth1 as output interface.\n");
  */
}

void parse_command_line(int argc, char* argv[])
{
	static struct option long_options[] = {
		{"help",       0, 0, 'h'},
		{"count",      1, 0, 'c'},
		{"file",       1, 0, 'f'},
		{"groupaddr",  1, 0, 'G'},
		{"rate",       1, 0, 'r'},
		{"outputif",   1, 0, 'o'},
		{"inputif",    1, 0, 'i'},
		{"daemon",     0, 0, 'D'},
		{"multiplex",  0, 0, 'M'},
		{"listen",     2, 0, 'L'},
		{"sleep",      2, 0, 'S'},
		{"raw",        2, 0, 'R'},
		{"udp",        0, 0, 'u'},		
		{"deliver",    0, 0, 'd'},
		{"hex", 	   0, 0,   0},
		{"loop",       0, 0,   0},
		{"saddr",      1, 0,   0},
		{"saddrlist",  1, 0,   0},
		{"sport",      1, 0,   0},
		{"ttl",        1, 0,   0},
		{"syslog",     1, 0,   0},
		{"threadpool", 2, 0,   0},
		{0, 0, 0, 0}
	};

	const char* optlist = "hvuDc:d:f:G:i:o:r:ML::S::R::";
	char *p, temp[128] = "\0";

	while (true){
		int index = 0;
		int c = getopt_long(argc, argv, optlist, long_options, &index);
		if (c == EOF) break;
		switch (c) {
		case 'v':
			gl_verbose_enable = true;
			break;
		case 'u':
			gl_listen_mode |= UDPCLIENT;
			break;
		case 'd':
			string_setting(optarg, &gl_sendbuffer);
			break;
		case 'D':
			gl_daemon_enable = true;
			break;
		case 'M':
			gl_listen_mode |= MULTIPLEX;
			break;
		case 'L':
			gl_listen_mode = TCPSERVER;
			if (optarg != NULL) {
				if (strcmp (optarg, "udp") == 0) 
					gl_listen_mode = UDPSERVER;
				else if (strcmp (optarg, "multicast") == 0) {
					gl_listen_mode = UDPSERVER | MULTICAST;
				}
				else goto error;
			}
			break;
		case 'S':
			gl_spec_sleep = true;
			if (optarg != NULL)
				gl_sleep_usec = atoi(optarg);
			break;
		case 'R':
			gl_listen_mode |= RAW_IP_DATAGRAM;
			break;			
		case 'c':
			gl_send_count = atoi(optarg);
			break;
		case 'f':
			string_setting(optarg, &gl_sendfilename);
			break;
		case 'G':
			string_setting(optarg, &gl_ipmulti_groupaddr);
			break;
		case 'i':
			string_setting(optarg, &gl_ipmulti_inputif);
			break;
		case 'o':
			string_setting(optarg, &gl_ipmulti_outputif);
			break;
		case 'r':
			gl_rete_control_enable = true;
			strcpy (temp, optarg);
			if (isdigit (temp[strlen(temp)-1])){
				gl_send_rate= atoi(temp);
			} else {
				if (temp[strlen(temp)-1] == 'K' || temp[strlen(temp)-1] == 'k') {
					temp[strlen(temp)-1] = 0;
					gl_send_rate = static_cast<int> (1024*atof(temp));
				}
				else if  (temp[strlen(temp)-1] == 'M' || temp[strlen(temp)-1] == 'm') {
					temp[strlen(temp)-1] = 0;
					gl_send_rate = static_cast<int> (1024*1024*atof(temp));
				}
				else {
					gl_rete_control_enable = false;
				}
			}
			break;
		case 0: // only long options
		if (strcmp ("saddr", long_options[index].name) == 0)
			string_setting(optarg, &gl_source_ipaddress);
		else if (strcmp ("saddrlist", long_options[index].name) == 0)
			string_setting(optarg, &gl_source_ipaddresses);
		else if (strcmp ("sport", long_options[index].name) == 0)
			gl_source_port = atoi(optarg);
		else if (strcmp ("ttl", long_options[index].name) == 0)
			gl_ipmulti_ttl = atoi(optarg);
		else if (strcmp ("hex", long_options[index].name) == 0)
			gl_process_hex_stdin = true;
		else if (strcmp ("threadpool", long_options[index].name) == 0){
			gl_listen_mode |= THREADPOOL;
			if (optarg != NULL)
				gl_thread_pool_size = atoi(optarg);
		}
		break;
		case 'h':
		default :
			usage ();
			exit(0);
		}
	}

	// End Prase command line
	///////////////////////////////////////////////////////////
	if ((gl_listen_mode&MULTICAST) == MULTICAST)
		if (gl_ipmulti_groupaddr == NULL)
			goto error;
	if (gl_listen_mode != TCPCLIENT && gl_listen_mode != UDPCLIENT)
		return ;

	strcpy (temp, argv[argc-1]);
	p = strpbrk (temp, ":");
	if (p != NULL){
		*p = 0;
		gl_destination_port = atoi(p+1); 
		string_setting(temp, &gl_destination_ipaddress);
	}
	else goto error; 
	return;

error:
	usage ();
	exit(0);
}

