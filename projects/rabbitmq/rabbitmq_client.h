#ifndef RABBITMQ_CLIENT_H__
#define RABBITMQ_CLIENT_H__

#include <stdint.h>


/**
 * Create a handle and connect to RabbitMQ server specified by @url, the
 * channel specified by @channel. In AMQP model channels can be thought of
 * as"lightwight connections that share a single TCP connection"
 * Just set @channel to 0 and let system allocate channel id automatically
 *
 *
 * URL = scheme://[username:password]@hostname[:port][/vhost]?parameters
 *
 * DEFAULT username: guest
 * DEFAULT password: guest
 * DEFAULT hostname: localhost
 * DEFAULT port:     5672
 * DEFAULT vhost:    /

 * URL parameters
 *   Name       Type         Desc
 *   queue      string       specific the queue name, default log.queue
 *   ex         string       exchange name, default log.direct
 *   bind       string       binding key, default log.binding
 * example url: amqp://guest:guest@localhost:5672/ck?queue=log.queue&ex=direct
 */

void *amq_create_consumer(const char *url, uint16_t channel);
void *amq_create_producer(const char *url, uint16_t channel);

/**
 * Destroy handle @hp that create by above APIs
 */
void amq_destroy(void *hp);

/**
 * Send function
 */
int amq_send(void *hp, void *data, size_t len);


/**
 * Recv function
 */
typedef int (*process_func)(void *data, size_t len);
int amq_recv_loop(void *hp, process_func callback);



#endif
