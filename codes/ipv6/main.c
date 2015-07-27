#include "ping6.h"



int main(int argc, char *argv[])
{
	ipv6_command_line(argc, argv);

	if (strcmp(config.command, "ping") == 0) {
		i6ping(config.host);
	} else if (strcmp(config.command, "tracert") == 0) {
	} else if (strcmp(config.command, "nc") == 0) {
	}

	return 0;
}

