#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <hexdump.h>

#include "rabbitmq_client.h"

char xdump[409600];
int process_message(void *data, size_t len)
{
    printf("receive %zu bytes message: \n%s\n", len,
           hexdump(HEXDUMP_C, data, len, xdump));
    return 0;
}

int main(int argc, char *argv[])
{
    int c;
    char url[1024];
    const char *ex = NULL;
    const char *binding = NULL;
    const char *queue = NULL;

    while ((c = getopt(argc, argv, "b:e:q:")) != -1) {
        switch(c) {
        case 'b':
            binding = strdup(optarg);
            break;
        case 'e':
            ex = strdup(optarg);
            break;
        case 'q':
            queue = strdup(optarg);
        default:
            break;
        }
    }

    snprintf(url, sizeof(url), "amqp://localhost/?ex=%s&bind=%s&queue=%s",
             ex ? ex : "",
             binding? binding : "",
             queue ? queue : "");

    void *handle = amq_create_consumer(url, 0);
    if (!handle) {
        return 1;
    }
    amq_recv_loop(handle, process_message);
    amq_destroy(handle);

    return 0;
}
