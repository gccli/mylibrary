#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <assert.h>

#include "rabbitmq_client.h"


int process_message(void *data, size_t len)
{
    printf("receive %zu bytes message: \n", len);

    return 0;
}

int main(int argc, char const *const *argv)
{
    const char *url = "amqp://guest:guest@localhost:5672/?queue=log.queue&ex="
        "log.direct&bind=log.binding";
    void *handle = amq_create_consumer(url, 1);
    if (!handle) {
        return 1;
    }
    amq_recv_loop(handle, process_message);
    amq_destroy(handle);

    return 0;
}
