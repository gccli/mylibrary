#ifndef RABBITMQ_CLIENT_H__
#define RABBITMQ_CLIENT_H__

#include <stdint.h>

/**
 * Create a handle and connect to RabbitMQ server specified by @url, the
 * channel specified by @channel. In AMQP model channels can be thought of
 * as"lightwight connections that share a single TCP connection"
 * Just set @channel to 0 and let system allocate channel id automatically
 *
 * URL = scheme://[username:password]@hostname[:port][/vhost]?parameters
 * scheme always amqp
 *  username: RabbitMQ username, default guest
 *  password: password for username, default guest
 *  hostname: RabbitMQ server host, default localhost
 *  port:     RabbitMQ server host, default 5672
 *  vhost:    Virtual Hosts, default "/"
 *
 * parameters = key1=value1&key2=value2&...&keyN=valueN, valid key as follow:
 *   Name     Type     Desc
 *   queue    string   Queue used forstore messages, only consumer need, if set
 *                     empty RabbitMQ will generate it
 *   ex       string   Exchange name, used for route nessages into queues,
 *                     default "amq.direct"
 *   bind     string   A binding is a relationship between an exchange and
 *                     a queue, default "log"
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
