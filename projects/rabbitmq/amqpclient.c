#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <amqp_tcp_socket.h>
#include <amqp.h>
#include <amqp_framing.h>

#include "amqpclient.h"

#define DEFAULT_VHOST        "/ck"
#define DEFAULT_QUEUE        "log_queue"
#define DEFAULT_EXCHANGE     "log_exchange"
#define DEFAULT_BINDING      "log_binding"


#define DEFAULT_RABBIT_HOST  "localhost"
#define DEFAULT_RABBIT_PORT   5672

#define MAX_QUEUE_NAME_LEN 32
#define MAX_VHOST_NAME_LEN 32
#define MAX_EXCHANGE_NAME_LEN 32
typedef struct amq_client {
    amqp_connection_state_t conn;
    amqp_channel_t channel;
} amq_client_t;


void print_amqp_error(amqp_rpc_reply_t x, char const *context)
{
  switch (x.reply_type) {
  case AMQP_RESPONSE_NORMAL:
    return;

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
              context,
              m->reply_code,
              (int) m->reply_text.len, (char *) m->reply_text.bytes);
      break;
    }
    case AMQP_CHANNEL_CLOSE_METHOD: {
      amqp_channel_close_t *m = (amqp_channel_close_t *) x.reply.decoded;
      fprintf(stderr, "%s: server channel error %d, message: %.*s\n",
              context,
              m->reply_code,
              (int) m->reply_text.len, (char *) m->reply_text.bytes);
      break;
    }
    default:
      fprintf(stderr, "%s: unknown server error, method id 0x%08X\n", context, x.reply.id);
      break;
    }
    break;
  }

  exit(1);
}


static amqp_bytes_t default_queue = {strlen(DEFAULT_QUEUE), DEFAULT_QUEUE};
static amqp_bytes_t default_exchange = {strlen(DEFAULT_EXCHANGE), DEFAULT_EXCHANGE};

//
// Connect to localhost:5672 as guest with the password guest and virtual host "/ck"
// example : amqp://guest:guest@localhost:5672/ck?q=log_queue&exchange=log_exchange')
//
void *amq_client_create(const char *url)
{
    int status;
    amq_client_t *handle;

    amqp_rpc_reply_t x;
    char hostname[128] = "localhost";
    int port = DEFAULT_RABBIT_PORT;

    amqp_socket_t *socket = NULL;
    amqp_queue_declare_ok_t *r;

    handle = (amq_client_t *)calloc(1, sizeof(*handle));

    handle->conn = amqp_new_connection();
    handle->channel = 1;

    socket = amqp_tcp_socket_new(handle->conn);
    status = amqp_socket_open(socket, hostname, port);
    if (status) {
        goto error;
    }

    x = amqp_login(handle->conn, DEFAULT_VHOST, 0, 65535, 0, AMQP_SASL_METHOD_PLAIN,
                   "guest", "guest");
    print_amqp_error(x, "Login");
    amqp_channel_open(handle->conn, handle->channel);
    x = amqp_get_rpc_reply(handle->conn);
    print_amqp_error(x, "Opening channel");

    r = amqp_queue_declare(handle->conn, handle->channel, default_queue, 0, 0, 0,
                           1, amqp_empty_table);
    x = amqp_get_rpc_reply(handle->conn);
    print_amqp_error(x, "Declaring queue");


    amqp_queue_bind(handle->conn, handle->channel, default_queue, default_exchange,
                    amqp_cstring_bytes(DEFAULT_BINDING),
                    amqp_empty_table);
    x = amqp_get_rpc_reply(handle->conn);
    print_amqp_error(x, "Binding queue");

    return handle;

error:
    if (handle) {
        free(handle);
    }
    return NULL;
}
