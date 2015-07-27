#include <getopt.h>
#include "config.h"

struct ipv6_config config;

static void usage()
{
	printf("Usage ipv6 [ -Dv ] [ -c count ] [ -I ifname ]\n");
	printf("           command host \n");
	exit(0);
}

void ipv6_command_line(int argc, char *argv[])
{
	if (argc < 3) {
		usage();
	}

	memset(&config, 0, sizeof(config));
	int c;
	while (1) {
		int option_index = 0;	
		if ((c = getopt_long(argc, argv, "c:t:I:Dv", NULL, &option_index)) < 0)
			break;
		switch (c) {
		case 'c':
			config.count = atoi(optarg);
			break;
		case 't':
			config.ttl = atoi(optarg);
			break;			
		case 'D':
			config.debug = 1;
			break;
		case 'I':
			strcpy(config.ifname,optarg);
			if (config.debug) {
				printf("The index of %s is %d\n", config.ifname, if_nametoindex(config.ifname));
			}
			break;
		case 'v':
			config.verbose++;
			break;
		default:
			usage();
		}
	}

	strcpy(config.command, argv[argc-2]);
	strcpy(config.host, argv[argc-1]);
	if (config.debug) {
		printf("argc %d command [%s] host [%s]\n", argc, config.command, config.host);
	}
}

