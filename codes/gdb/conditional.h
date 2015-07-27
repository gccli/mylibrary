#ifndef __CONDITIONAL_H__
#define __CONDITIONAL_H__


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef _WIN32
#include <Winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#else
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#endif
