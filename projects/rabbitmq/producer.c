#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "rabbitmq_client.h"

int main(int argc, char *argv[])
{
    int i;
    const char *url = "amqp://guest:guest@localhost:5672/?queue=log.queue&ex="
        "log.direct&bind=log.binding";

    void *handle = amq_create_producer(url, 1);
    if (!handle) {
        return 1;
    }

    char buff[4096];
    for(i=0; i<20; ++i) {
        amq_send(handle, buff, sizeof(buff));
    }

    amq_destroy(handle);

    return 0;
}
