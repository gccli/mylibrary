#ifndef __DAC__H__
#define __DAC__H__

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <sys/time.h>

#include <pthread.h>
#include "daccommon.h"
#include "daccomm.h"

struct SAppEvnet
{
    int            msgLen;
    unsigned char *msgPtr; 
};

typedef int (*QUEUE_PROCESS_FUNC)(unsigned char *message, int len);

#endif

