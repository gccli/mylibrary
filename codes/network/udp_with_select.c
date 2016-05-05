#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <net/if.h>

#include <netdb.h>
#include <pthread.h>
#include <sys/time.h>

#include <assert.h>

int main(int argc, char *argv[])
{
    int    i, j, len, enable, count, port;
    int    sock[2];
    char   buffer[1024];
    fd_set rset;
    struct timeval     tv;
    struct sockaddr_in addr;

    for(i=0, port=10086; i<2; ++i, ++port) {
        sock[i] = socket(AF_INET, SOCK_DGRAM, 0);
        assert(sock[i] > 0);
        enable = 1;

	setsockopt(sock[i], SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        assert(bind(sock[i], (struct sockaddr *)&addr, sizeof(addr)) == 0);
    }

    while (1) {
        FD_ZERO(&rset);
        FD_SET(sock[0], &rset);
        FD_SET(sock[1], &rset);

        tv.tv_sec = 5; tv.tv_usec = 0;
        count = select(sock[1]+1, &rset, NULL, NULL, &tv);
        assert(count >= 0);
        if (count == 0) {
            continue;
        }
        printf("\n%d fds are ready to read\n", count);
        for(j=0; j<2; ++j) {
            if (!FD_ISSET(sock[j], &rset)) continue;

            len = recvfrom(sock[j], buffer, sizeof(buffer), 0, NULL, NULL);
            assert (len >= 0);
            buffer[len] = 0;
            printf("recv form sock[%d]:%s", j, buffer);
        }
    }

    return 0;
}
