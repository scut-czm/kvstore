
#ifndef _KVSTORE_H_
#define _KVSTORE_H_

#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
// tcp

#define BUFFER_LENGTH 1024

#define ENABLE_HTTP_RESPONSE 0

typedef int (*RCALLBACK)(int fd);

typedef struct conn_item {
  int fd;
  /*char buffer[BUFFER_LENGTH];*/
  /*int idx;*/

  char rbuffer[BUFFER_LENGTH];
  int rlen;
  char wbuffer[BUFFER_LENGTH];
  int wlen;

  union {
    RCALLBACK accept_callback;
    RCALLBACK recv_callback;
  } recv_t;
  RCALLBACK send_callback;
} connection_t;

int epoll_entry(void);
int ntyco_entry(void);
int kvstore_request(connection_t *item);

#define NETWORK_EPOLL 0
#define NETWORK_NTYCO 1
#define NETWORK_IOURING 2
#define ENABLE_NETWORK_SELECT NETWORK_NTYCO

#define ENABLE_ARRAY_KVENGINE 1

#ifdef ENABLE_ARRAY_KVENGINE

typedef struct kvs_array_item_s {
  char *key;
  char *value;
} kvs_array_item_t;

#define KVS_ARRAY_SIZE 1024
int kvstore_array_set(char *key, char *value);

char *kvstore_array_get(char *key);
extern kvs_array_item_t array_table[KVS_ARRAY_SIZE];
extern int array_idx;
#endif

void *kvstore_malloc(size_t size);

void kvstore_free(void *ptr);

#endif // _KVSTORE_H_
