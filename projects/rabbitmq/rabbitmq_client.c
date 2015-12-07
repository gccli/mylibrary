#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/time.h>

#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "rabbitmq_client.h"
#include "http_data.h"

#define DEFAULT_VHOST             "/"
#define DEFAULT_EXCHANGE          "amq.direct"
#define DEFAULT_BINDING           "log"
#define DEFAULT_EXCHTYPE          "direct"

#define DEFAULT_RABBITMQ_HOST       "localhost"
#define DEFAULT_RABBITMQ_PORT       5672
#define DEFAULT_RABBITMQ_USER       "guest"
#define DEFAULT_RABBITMQ_PASSWD     "guest"

#define DEFAULT_TIMEOUT           3

#define MAX_NAME_LEN              32
#define MAX_HOSTNAME_LEN          64

typedef struct amq_client {
    amqp_connection_state_t conn;
    amqp_channel_t channel;


    int port;                      // rabbitmq port, default 5672
    char host[MAX_HOSTNAME_LEN];   // rabbitmq hostname, default localhost
    char vhost[MAX_NAME_LEN];
    char queue[MAX_NAME_LEN];      // queue name
    char binding[MAX_NAME_LEN];    // binding name
    char exchange[MAX_NAME_LEN];   // exchange name
} amq_client_t;

static void print_amqp_error(amqp_rpc_reply_t x, char const *context)
{
    switch (x.reply_type) {
    case AMQP_RESPONSE_NORMAL:
        break;
    case AMQP_RESPONSE_NONE:
        fprintf(stderr, "%s: missing RPC reply type!\n", context);
        break;

    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
        fprintf(stderr, "%s: %s\n", context, amqp_error_string2(x.library_error));
        break;

    case AMQP_RESPONSE_SERVER_EXCEPTION:
        switch (x.reply.id) {
        case AMQP_CONNECTION_CLOSE_METHOD: {
            amqp_connection_close_t *m = (amqp_connection_close_t *) x.reply.decoded;
            fprintf(stderr, "%s: server connection error %d, message: %.*s\n",
                    context, m->reply_code,
                    (int) m->reply_text.len, (char *) m->reply_text.bytes);
            break;
        }
        case AMQP_CHANNEL_CLOSE_METHOD: {
            amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
            fprintf(stderr, "%s: server channel error %d, message: %.*s\n",
                    context, m->reply_code,
                    (int) m->reply_text.len, (char *) m->reply_text.bytes);
            break;
        }
        default:
            fprintf(stderr, "%s: unknown server error, method id 0x%08X\n",
                    context, x.reply.id);
            break;
        }
        break;
    }
}

#define CHECK_REPLY(x, context)                         \
    if ((x).reply_type != AMQP_RESPONSE_NORMAL) {       \
        print_amqp_error((x), context);                 \
        goto error;                                     \
    }

void amq_destroy(void *hp)
{
    amq_client_t *handle = (amq_client_t *) hp;
    if (handle) {
        if (handle->conn) {
            amqp_channel_close(handle->conn, handle->channel, AMQP_REPLY_SUCCESS);
            amqp_connection_close(handle->conn, AMQP_REPLY_SUCCESS);
            amqp_destroy_connection(handle->conn);
        }
        free(handle);
    }
}

static int amq_set_handle_params(amq_client_t *handle, const urlparser_t *u)
{
    int ret;
    size_t i, len;
    const char *key, *val;
    char tmps[MAX_NAME_LEN] = {0}, *endptr;
    ret = EINVAL;


    handle->port = DEFAULT_RABBITMQ_PORT;
    if ((len = u->port.len) > 0) {
        if (len > sizeof(tmps)) {
            printf("host %.*s too long\n", (int)len, u->port.data);
            goto error;
        }
        strncpy(tmps, u->port.data, len);

        handle->port = strtol(tmps, &endptr, 10);
        if (errno == ERANGE || endptr == tmps) {
            printf("port invalid %s\n", tmps);
            goto error;
        }
    }

    if ((len = u->host.len) > 0) {
        if (len > sizeof(handle->host)) {
            printf("host %.*s too long\n", (int)len, u->path.data);
            goto error;
        }
        strncpy(handle->host, u->host.data, len);
    } else {
        strcpy(handle->vhost, DEFAULT_RABBITMQ_HOST);
    }

    if ((len = u->path.len) > 0) {
        if (len > sizeof(handle->vhost)) {
            printf("vhost %.*s too long\n", (int)len, u->path.data);
            goto error;
        }
        strncpy(handle->vhost, u->path.data, len);
    } else {
        strcpy(handle->vhost, DEFAULT_VHOST);
    }

    for (i=0; i<u->params.count; i++) {
        key = u->params.kv[i].key;
        val = u->params.kv[i].val;

        if (key == NULL || *key == 0 || val == NULL || *val == 0)
            continue;
        len = strlen(val);
        if (len > MAX_NAME_LEN) {
            printf("%s = %s too long\n", key, val);
            goto error;
        }
        if (strcmp(key, "queue") == 0) {
            snprintf(handle->queue, MAX_NAME_LEN, "%s", val);
        } else if (strcmp(key, "ex") == 0) {
            snprintf(handle->exchange, MAX_NAME_LEN, "%s", val);
        } else if (strcmp(key, "bind") == 0) {
            snprintf(handle->binding, MAX_NAME_LEN, "%s", val);
        }
    }

    if (handle->exchange[0] == 0) {
        snprintf(handle->exchange, MAX_NAME_LEN, "%s", DEFAULT_EXCHANGE);
    }
    if (handle->binding[0] == 0) {
        snprintf(handle->binding, MAX_NAME_LEN, "%s", DEFAULT_BINDING);
    }
    ret = 0;

error:
    return ret;
}

static amq_client_t *amq_open_channel(const char *url, uint16_t channel)
{
    int status, err;
    size_t len;
    amq_client_t *handle;
    char user[MAX_NAME_LEN];
    char passwd[MAX_NAME_LEN];

    urlparser_t urlp;
    struct timeval tv;
    amqp_rpc_reply_t x;
    amqp_socket_t *socket = NULL;

    err = EINVAL;

    if (parse_url(url, strlen(url), &urlp, 1) != 0) {
        printf("URL [%s] is invalid\n", url);
        return NULL;
    }

    handle = (amq_client_t *)calloc(1, sizeof(*handle));
    if (handle == NULL) {
        goto error;
    }

    if (amq_set_handle_params(handle, &urlp)) {
        goto error;
    }

    handle->channel = channel;
    handle->conn = amqp_new_connection();

    // Create Socket to connect RabbitMQ host:port
    tv.tv_sec = DEFAULT_TIMEOUT;
    tv.tv_usec = 0;

    socket = amqp_tcp_socket_new(handle->conn);
    status = amqp_socket_open_noblock(socket, handle->host, handle->port, &tv);
    if (status) {
        printf("Open sock to %s:%d\n", handle->host, handle->port);
        goto error;
    }

    // Login
    // Set username, password, vhost
    if ((len = urlp.username.len) > 0) {
        if (len > sizeof(user)) {
            printf("username %.*s too long\n", (int)len, urlp.username.data);
            goto error;
        }
        strncpy(user, urlp.username.data, len);
    } else {
        strcpy(user, DEFAULT_RABBITMQ_USER);
    }
    if ((len = urlp.password.len) > 0) {
        if (len > sizeof(passwd)) {
            printf("password %.*s too long\n", (int)len, urlp.password.data);
            goto error;
        }
        strncpy(passwd, urlp.password.data, len);
    } else {
        strcpy(passwd, DEFAULT_RABBITMQ_PASSWD);
    }
    x = amqp_login(handle->conn, handle->vhost, 0, 131072, 0,
                   AMQP_SASL_METHOD_PLAIN,
                   user, passwd);
    CHECK_REPLY(x, "Login");

    // Open Channel
    amqp_channel_open(handle->conn, handle->channel);
    CHECK_REPLY(amqp_get_rpc_reply(handle->conn), "Opening channel");

    err = 0;
    printf("open channel success\n");

error:
    if (err && handle) {
        if (handle->conn) {
            amqp_channel_close(handle->conn, handle->channel, AMQP_REPLY_SUCCESS);
            amqp_connection_close(handle->conn, AMQP_REPLY_SUCCESS);
            amqp_destroy_connection(handle->conn);
        }
        free(handle); handle = NULL;
    }
    return handle;
}


void *amq_create_consumer(const char *url, uint16_t channel)
{
    int err;
    amq_client_t *handle;

    amqp_rpc_reply_t x;
    amqp_queue_declare_ok_t *r;

    err = EINVAL;
    handle = amq_open_channel(url, channel);
    if (handle == NULL) {
        return NULL;
    }

    // Declare Exchange

    if (strcmp(handle->exchange, DEFAULT_EXCHANGE) != 0) {
        amqp_exchange_declare(handle->conn, handle->channel,
                              amqp_cstring_bytes(handle->exchange),
                              amqp_cstring_bytes(DEFAULT_EXCHTYPE),
                              0, // passsive
                              0, // durable
                              1, // auto_delete
                              0, // intenal
                              amqp_empty_table);
        CHECK_REPLY(amqp_get_rpc_reply(handle->conn), "Declaring exchange");
    }
    // Declare Queue
    r = amqp_queue_declare(handle->conn, handle->channel,
                           amqp_cstring_bytes(handle->queue),
                           0, // passive
                           0, // durable
                           0, // exclusive
                           1, // auto_delete
                           amqp_empty_table);
    x = amqp_get_rpc_reply(handle->conn);
    CHECK_REPLY(x, "Declaring queue");

    // Binding Queue to Exchange
    amqp_queue_bind(handle->conn, handle->channel,
                    r->queue,
                    amqp_cstring_bytes(handle->exchange),
                    amqp_cstring_bytes(handle->binding),
                    amqp_empty_table);
    x = amqp_get_rpc_reply(handle->conn);
    CHECK_REPLY(x, "Binding queue");

    amqp_basic_consume(handle->conn, handle->channel,
                       r->queue,
                       amqp_empty_bytes, // tag
                       0,  // no_local
                       1,  // no_ack
                       0,  // exclusive
                       amqp_empty_table);
    x = amqp_get_rpc_reply(handle->conn);
    CHECK_REPLY(x, "Binding queue");
    err = 0;

error:
    if (err && handle) {
        amq_destroy(handle);
        handle = NULL;
    }
    return handle;
}

void *amq_create_producer(const char *url, uint16_t channel)
{
    return amq_open_channel(url, channel);
}

int amq_send(void *hp, void *data, size_t len)
{
    int err;
    amqp_bytes_t message;
    amq_client_t *handle = (amq_client_t *) hp;

    message.len = len;
    message.bytes = data;

    err = amqp_basic_publish(handle->conn,
                             handle->channel,
                             amqp_cstring_bytes(handle->exchange),
                             amqp_cstring_bytes(handle->binding),
                             0,    // mandatory
                             0,    // immediate
                             NULL, // properties, e.g. content-type
                             message);
    if (err < 0) {
        fprintf(stderr, "Publishing: %s\n", amqp_error_string2(err));
        return EIO;
    }

    printf("publish return %d\n", err);

    return 0;
}

int amq_recv_one(amqp_connection_state_t conn, process_func callback)
{
    amqp_rpc_reply_t ret;
    amqp_envelope_t envelope;
    amqp_frame_t frame;

    amqp_maybe_release_buffers(conn);

    ret = amqp_consume_message(conn, &envelope, NULL, 0);

    if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
        if (AMQP_RESPONSE_LIBRARY_EXCEPTION == ret.reply_type &&
            AMQP_STATUS_UNEXPECTED_STATE == ret.library_error) {
            if (AMQP_STATUS_OK != amqp_simple_wait_frame(conn, &frame)) {
                return EINVAL;
            }

            if (AMQP_FRAME_METHOD == frame.frame_type) {
                switch (frame.payload.method.id) {
                case AMQP_BASIC_ACK_METHOD:
                    /* if we've turned publisher confirms on, and we've published a message
                     * here is a message being confirmed
                     */
                    break;
                case AMQP_BASIC_RETURN_METHOD:
                    /* if a published message couldn't be routed and the mandatory flag was set
                     * this is what would be returned. The message then needs to be read.
                     */
                {
                    amqp_message_t message;
                    ret = amqp_read_message(conn, frame.channel, &message, 0);
                    if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
                        return EINVAL;
                    }

                    amqp_destroy_message(&message);
                }
                break;

                case AMQP_CHANNEL_CLOSE_METHOD:
                    /* a channel.close method happens when a channel exception occurs, this
                     * can happen by publishing to an exchange that doesn't exist for example
                     *
                     * In this case you would need to open another channel redeclare any queues
                     * that were declared auto-delete, and restart any consumers that were attached
                     * to the previous channel
                     */
                    return EIO;

                case AMQP_CONNECTION_CLOSE_METHOD:
                    /* a connection.close method happens when a connection exception occurs,
                     * this can happen by trying to use a channel that isn't open for example.
                     *
                     * In this case the whole connection must be restarted.
                     */
                    return EIO;
                default:
                    fprintf(stderr ,"An unexpected method was received %d\n", frame.payload.method.id);
                    return EINVAL;
                }
            }
        }
    } else {
        callback(envelope.message.body.bytes, envelope.message.body.len);
        amqp_destroy_envelope(&envelope);
    }

    return 0;
}

int amq_recv_loop(void *hp, process_func callback)
{
    amq_client_t *handle = (amq_client_t *) hp;
    while(1) {
        if (amq_recv_one(handle->conn, callback) != 0) {
            printf("error in receive one message\n");
            break;
        }
    }

    return 0;
}
