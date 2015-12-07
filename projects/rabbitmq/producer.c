#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include "rabbitmq_client.h"

int main(int argc, char *argv[])
{
    int i, c;
    char url[1024] = {0};

    const char *ex = NULL;
    const char *binding = NULL;

    while ((c = getopt(argc, argv, "b:e:")) != -1) {
        switch(c) {
        case 'b':
            binding = strdup(optarg);
            break;
        case 'e':
            ex = strdup(optarg);
            break;
        default:
            break;
        }
    }

    snprintf(url, sizeof(url), "amqp://localhost/?ex=%s&bind=%s&",
             ex ? ex : "",
             binding? binding : "");

    void *handle = amq_create_producer(url, 1);
    if (!handle) {
        return 1;
    }

    for (i=optind; argv[i]; i++) {
        amq_send(handle, argv[i], strlen(argv[i]));
    }

    amq_destroy(handle);

    return 0;
}
