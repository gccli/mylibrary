#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#include <zookeeper.h>
#include <proto.h>

#define _LL_CAST_ (long long)

static zhandle_t *zh;
static clientid_t myid;
static const char *host;
static int shutdownThisThing=0;
static int to_send=0;
static int sent=0;
static int recvd=0;

static const char* state2String(int state){
  if (state == 0)
    return "CLOSED_STATE";
  if (state == ZOO_CONNECTING_STATE)
    return "CONNECTING_STATE";
  if (state == ZOO_ASSOCIATING_STATE)
    return "ASSOCIATING_STATE";
  if (state == ZOO_CONNECTED_STATE)
    return "CONNECTED_STATE";
  if (state == ZOO_EXPIRED_SESSION_STATE)
    return "EXPIRED_SESSION_STATE";
  if (state == ZOO_AUTH_FAILED_STATE)
    return "AUTH_FAILED_STATE";

  return "INVALID_STATE";
}
static const char* type2String(int state){
  if (state == ZOO_CREATED_EVENT)
    return "CREATED_EVENT";
  if (state == ZOO_DELETED_EVENT)
    return "DELETED_EVENT";
  if (state == ZOO_CHANGED_EVENT)
    return "CHANGED_EVENT";
  if (state == ZOO_CHILD_EVENT)
    return "CHILD_EVENT";
  if (state == ZOO_SESSION_EVENT)
    return "SESSION_EVENT";
  if (state == ZOO_NOTWATCHING_EVENT)
    return "NOTWATCHING_EVENT";

  return "UNKNOWN_EVENT_TYPE";
}

static const char *zoo_errors(int rc) {
  static char error[128] = {0};
  switch (rc) {
    case ZOK:
      sprintf(error, "OK"); break;
    case ZAPIERROR:
      sprintf(error, "%d API Error", rc); break;
    case ZNONODE:
      sprintf(error, "%d Node does not exist", rc); break;
    case ZNODEEXISTS:
      sprintf(error, "%d The node already exists", rc); break;
    case ZNOTEMPTY:
      sprintf(error, "%d The node has children", rc); break;
    default:
      sprintf(error, "%d Unknown", rc); break;
  }
  return error;
}

#define RETURN_IF_FAIL(rc)				\
  if (rc)  {						\
    fprintf(stderr, "%s\n", zoo_errors(rc));		\
    return ;						\
  }

void watcher(zhandle_t *zzh, int type, int state, const char *path,
             void* context)
{
    /* Be careful using zh here rather than zzh - as this may be mt code
     * the client lib may call the watcher before zookeeper_init returns */

    fprintf(stderr, "Watcher %s state = %s", type2String(type), state2String(state));
    if (path && strlen(path) > 0) {
      fprintf(stderr, " for path %s", path);
    }
    fprintf(stderr, "\n");

    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            const clientid_t *id = zoo_client_id(zzh);
            if (myid.client_id == 0 || myid.client_id != id->client_id) {
                myid = *id;
                fprintf(stderr, "Got a new session id: 0x%llx\n", _LL_CAST_ myid.client_id);
            }
        } else if (state == ZOO_AUTH_FAILED_STATE) {
            fprintf(stderr, "Authentication failure. Shutting down...\n");
            zookeeper_close(zzh);
            shutdownThisThing=1;
            zh=0;
        } else if (state == ZOO_EXPIRED_SESSION_STATE) {
            fprintf(stderr, "Session expired. Shutting down...\n");
            zookeeper_close(zzh);
            shutdownThisThing=1;
            zh=0;
        }
    }
}

static const char *dump_time(time_t t)
{
  static char outstr[32] = {0};
  struct tm tmp;
  localtime_r(&t, &tmp);
  if (strftime(outstr, sizeof(outstr), "%Y/%M/%d %T", &tmp) == 0) {
    return NULL;
  }
  return outstr;
}

void dump_stat(const struct Stat *stat) {
    if (!stat) {
        fprintf(stderr,"null\n");
        return;
    }
    fprintf(stderr, "\tctime = %s\tczxid=%llx\n"
	    "\tmtime = %s\tmzxid=%llx\n"
	    "\tversion=%x\taversion=%x\n"
	    "\tephemeralOwner = %llx\n",
	    dump_time(stat->ctime/1000), _LL_CAST_ stat->czxid,
	    dump_time(stat->mtime/1000),  _LL_CAST_ stat->mzxid,
	    (unsigned int)stat->version, (unsigned int)stat->aversion,
	    _LL_CAST_ stat->ephemeralOwner);
}

void my_string_completion(int rc, const char *name, const void *data) {
  RETURN_IF_FAIL(rc);
  fprintf(stderr, "[%s]: name = %s\n", (char *)data, name);
}

void my_data_completion(int rc, const char *value, int value_len,
			const struct Stat *stat, const void *data) {
  RETURN_IF_FAIL(rc);
  fprintf(stderr, "[%s]:\n", (char*)data);
  if (value) {
    fprintf(stderr, "%s (len = %d)\n", value, value_len);
  }
  fprintf(stderr, "\nStat:\n");
  dump_stat(stat);
  free((void*)data);
}

void my_strings_completion(int rc, const struct String_vector *strings,
			   const void *data) {
  RETURN_IF_FAIL(rc);
  int i;
  fprintf(stderr, "[%s]: \n", (char*)data);
  if (strings && strings->count > 0) {
    printf("[");
    for (i=0; i < strings->count-1; i++) {
      fprintf(stdout, "%s,", strings->data[i]);
    }
    printf("%s]\n", strings->data[i]);
  }
  free((void*)data);
}

void my_strings_stat_completion(int rc, const struct String_vector *strings,
        const struct Stat *stat, const void *data) {
  RETURN_IF_FAIL(rc);
  my_strings_completion(rc, strings, data);
  dump_stat(stat);
}

void my_string_completion_free_data(int rc, const char *name, const void *data) {
    my_string_completion(rc, name, data);
    free((void*)data);
}

void my_void_completion(int rc, const void *data) {
  RETURN_IF_FAIL(rc);
  fprintf(stderr, "[%s]\n", (char*)data);
  free((void*)data);
}

void my_stat_completion(int rc, const struct Stat *stat, const void *data) {
  RETURN_IF_FAIL(rc);
  fprintf(stderr, "[%s]:\n", (char*)data);
  dump_stat(stat);
  free((void*)data);
}

void my_silent_stat_completion(int rc, const struct Stat *stat,
        const void *data) {
    //    fprintf(stderr, "State completion: [%s] rc = %d\n", (char*)data, rc);
  sent++;
  free((void*)data);
}

void my_silent_data_completion(int rc, const char *value, int value_len,
        const struct Stat *stat, const void *data) {
    recvd++;
    fprintf(stderr, "Data completion %s rc = %d\n",(char*)data,rc);
    free((void*)data);
    if (recvd==to_send) {
        fprintf(stderr,"Recvd %d responses for %d requests sent\n",recvd,to_send);
    }
}

static void sendRequest(const char* data) {
  zoo_aset(zh, "/od", data, strlen(data), -1, my_silent_stat_completion, strdup("/od"));
  zoo_aget(zh, "/od", 1, my_silent_data_completion, strdup("/od"));
}

void od_completion(int rc, const struct Stat *stat, const void *data) {
    int i;
    RETURN_IF_FAIL(rc);
    fprintf(stderr, "Stat:\n");
    dump_stat(stat);
    // send a whole bunch of requests
    recvd=0;
    sent=0;
    to_send=200;
    for (i=0; i<to_send; i++) {
        char buf[4096*16];
        memset(buf, -1, sizeof(buf)-1);
        buf[sizeof(buf)-1]=0;
        sendRequest(buf);
    }
}

void trim(char *line) {
  char *p;
  size_t l = strlen(line);
  for(p = line+l-1; p-line >= 0; p--)
    if(*p == ' ' || *p == '\t') *p=0;
  for(p=line; p && *p; p++)
    if(*p == ' ' || *p == '\t') *p=0;
}

#define check_line(line)						\
  if (line[0] != '/') {							\
    fprintf(stderr, "Path must start with /, found: %s\n", line);	\
    return;								\
  }									\
  trim(line);

static inline int starts_with(const char *line, const char *prefix) {
    int len = strlen(prefix);
    return strncmp(line, prefix, len) == 0;
}

void processline(char *line) {
  int rc;
  if (starts_with(line, "help")) {
    fprintf(stderr, "    create [+[e|s]] <path>\n");
    fprintf(stderr, "    delete <path>\n");
    fprintf(stderr, "    set <path> <data>\n");
    fprintf(stderr, "    get <path>\n");
    fprintf(stderr, "    ls <path>\n");
    fprintf(stderr, "    ls2 <path>\n");
    fprintf(stderr, "    sync <path>\n");
    fprintf(stderr, "    exists <path>\n");
    fprintf(stderr, "    wexists <path>\n");
    fprintf(stderr, "    myid\n");
    fprintf(stderr, "    addauth <id> <scheme>\n");
    fprintf(stderr, "    quit\n");
  } else if (starts_with(line, "get ")) {
    line += 4; check_line(line);
    rc = zoo_aget(zh, line, 1, my_data_completion, strdup(line));
    assert(rc == 0);
  } else if (starts_with(line, "set ")) {
    line += 4; check_line(line);
    char *ptr = strchr(line, ' ');
    if (!ptr) {
      fprintf(stderr, "No data found after path\n");
      return;
    }
    *ptr++ = '\0';
    rc = zoo_aset(zh, line, ptr, strlen(ptr), -1, my_stat_completion, strdup(line));
    assert(rc == 0);
  } else if (starts_with(line, "ls ")) {
    line += 3; check_line(line);
    rc = zoo_aget_children(zh, line, 1, my_strings_completion, strdup(line));
    assert(rc == 0);
  } else if (starts_with(line, "ls2 ")) {
    line += 4; check_line(line);
    rc=zoo_aget_children2(zh, line, 1, my_strings_stat_completion, strdup(line));
    assert(rc == 0);
  } else if (starts_with(line, "create ")) {
    int flags = 0;
    line += 7;
    if (line[0] == '+') {
      line++;
      if (line[0] == 'e') {
	flags |= ZOO_EPHEMERAL;
	line++;
      }
      if (line[0] == 's') {
	flags |= ZOO_SEQUENCE;
	line++;
      }
      line++;
    }
    check_line(line);
    fprintf(stderr, "Creating [%s] node\n", line);
//        {
//            struct ACL _CREATE_ONLY_ACL_ACL[] = {{ZOO_PERM_CREATE, ZOO_ANYONE_ID_UNSAFE}};
//            struct ACL_vector CREATE_ONLY_ACL = {1,_CREATE_ONLY_ACL_ACL};
//            rc = zoo_acreate(zh, line, "new", 3, &CREATE_ONLY_ACL, flags,
//                    my_string_completion, strdup(line));
//        }
    rc = zoo_acreate(zh, line, "new", 3, &ZOO_OPEN_ACL_UNSAFE, flags, my_string_completion_free_data, strdup(line));
    assert(rc == 0);
  } else if (starts_with(line, "delete ")) {
    line += 7; check_line(line);
    rc = zoo_adelete(zh, line, -1, my_void_completion, strdup(line));
    assert(rc == 0);
  } else if (starts_with(line, "sync ")) {
    line += 5; check_line(line);
    rc = zoo_async(zh, line, my_string_completion_free_data, strdup(line));
    assert(rc == 0);
  } else if (starts_with(line, "wexists ")) {
    struct Stat stat;
    line += 8; check_line(line);
    rc = zoo_wexists(zh, line, watcher, (void*) 0, &stat);
    assert(rc == 0);
  } else if (starts_with(line, "exists ")) {
    struct Stat stat;
    line += 7; check_line(line);
    rc = zoo_exists(zh, line, 1, &stat);
    fprintf(stderr, "%s\n", zoo_errors(rc));
  } else if (strcmp(line, "myid") == 0) {
    printf("session Id = %llx\n", _LL_CAST_ zoo_client_id(zh)->client_id);
  } else if (strcmp(line, "reinit") == 0) {
    zookeeper_close(zh);
    // we can't send myid to the server here -- zookeeper_close() removes 
    // the session on the server. We must start anew.
    zh = zookeeper_init(host, watcher, 30000, 0, 0, 0);
  } else if (starts_with(line, "quit")) {
    fprintf(stderr, "Quitting...\n");
    shutdownThisThing=1;
  } else if (starts_with(line, "od")) {
    const char val[]="fire off";
    fprintf(stderr, "Overdosing...\n");
    rc = zoo_aset(zh, "/od", val, sizeof(val)-1, -1, od_completion, 0);
    assert(rc == 0);
  } else if (starts_with(line, "addauth ")) {
    char *ptr;
    line += 8;
    ptr = strchr(line, ' ');
    if (ptr) {
      *ptr = '\0';
      ptr++;
    }
    zoo_add_auth(zh, line, ptr, ptr ? strlen(ptr) : 0, NULL, NULL);
  }
}

int main(int argc, char *argv[])
{
  char buffer[4096];
  int  bufoff = 0;
  host = argv[1];
  if (!host) host = "localhost:2181";

  zoo_set_debug_level(ZOO_LOG_LEVEL_INFO);
  zoo_deterministic_conn_order(1); // enable deterministic order
  zh = zookeeper_init(host, watcher, 30000, &myid, 0, 0);
  if (!zh)
    return errno;

  sprintf(buffer, "ls /");
  processline(buffer);

  while(!shutdownThisThing) {
    int rc;
    int len = sizeof(buffer) - bufoff -1;
    if (len <= 0) {
      fprintf(stderr, "Can't handle lines that long!\n");
      exit(2);
    }
    rc = read(0, buffer+bufoff, len);
    if (rc <= 0) {
      fprintf(stderr, "bye\n");
      shutdownThisThing=1;
      break;
    }
    bufoff += rc;
    buffer[bufoff] = '\0';
    while (strchr(buffer, '\n')) {
      char *ptr = strchr(buffer, '\n');
      *ptr = '\0';
      processline(buffer);
      ptr++;
      memmove(buffer, ptr, strlen(ptr)+1);
      bufoff = 0;
    }
  }
  if (to_send!=0)
    fprintf(stderr,"Recvd %d responses for %d requests sent\n",recvd,sent);
  zookeeper_close(zh);
  
  return 0;
}
