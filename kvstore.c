#include "kvstore.h"

#define KVSTORE_MAX_TOKEN 128

const char *commands[] = {
    "SET",
    "GET",
    "DEL",
    "MOD",
};

enum {
  KVSTORE_CMD_START = 0,
  KVSTORE_CMD_SET = KVSTORE_CMD_START,
  KVSTORE_CMD_GET,
  KVSTORE_CMD_DEL,
  KVSTORE_CMD_MOD,
  KVSTORE_CMD_COUNT,
};

// 创建和销毁kvstore
void *kvstore_malloc(size_t size) { return malloc(size); }

void kvstore_free(void *ptr) { return free(ptr); }

int kvstore_split_token(char *msg, char **tokens) {
  if (msg == NULL || tokens == NULL)
    return -1;
  int idx = 0;
  char *token = strtok(msg, " ");
  while (token != NULL) {
    tokens[idx++] = token;
    token = strtok(NULL, " ");
  }
  return idx;
}

int kvstore_parser_protocol(connection_t *item, char **tokens, int count) {
  if (item == NULL || tokens == NULL || count <= 0) {
    return -1;
  }
  int cmd = KVSTORE_CMD_START;
  for (cmd = KVSTORE_CMD_START; cmd < KVSTORE_CMD_COUNT; cmd++) {
    if (strcmp(tokens[0], commands[cmd]) == 0) {
      break;
    }
  }
  printf("cmd: %d, count: %d\n", cmd, count);

  char *msg = item->wbuffer;
  memset(msg, 0, BUFFER_LENGTH);
  switch (cmd) {

  case KVSTORE_CMD_SET: {
    printf("set\n");
    int res = kvstore_array_set(tokens[1], tokens[2]);
    if (!res) {
      snprintf(msg, BUFFER_LENGTH, "SUCCESS");
    } else {
      snprintf(msg, BUFFER_LENGTH, "FAILURE");
    }
    break;
  }

  case KVSTORE_CMD_GET: {

    char *value = kvstore_array_get(tokens[1]);
    if (value != NULL) {
      snprintf(msg, BUFFER_LENGTH, "VALUE: %s", value);
    } else {
      snprintf(msg, BUFFER_LENGTH, "NOT EXIST");
    }
    break;
    printf("get: %s\n", value);
  }
  case KVSTORE_CMD_DEL:
    printf("del\n");
    break;
  case KVSTORE_CMD_MOD:
    printf("mod\n");
    break;
  default:
    printf("cmd: %s\n", tokens[0]);
    assert(0);
  }
}

// SET key value
int kvstore_request(connection_t *item) {
  printf("kvstore_request called for fd: %d\n", item->fd);
  printf("recv: %s\n", item->rbuffer);

  char *msg = item->rbuffer;
  char *tokens[KVSTORE_MAX_TOKEN];

  int token_count = kvstore_split_token(msg, tokens);
  int idx = 0;
  for (idx = 0; idx < token_count; idx++) {
    printf("idx: %d, token: %s\n", idx, tokens[idx]);
  }

  kvstore_parser_protocol(item, tokens, token_count);

  return 0;
}

int main() {
#if (ENABLE_NETWORK_SELECT == NEWORK_EPOLL)
  epoll_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_NTYCO)
  ntyco_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_IOURING)
#endif
}
