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
  LOG("cmd: %d, count: %d\n", cmd, count);

  char *msg = item->wbuffer;
  char *key = tokens[1];
  char *value = tokens[2];
  memset(msg, 0, BUFFER_LENGTH);
  switch (cmd) {

  case KVSTORE_CMD_SET: {
    int res = kvstore_array_set(key, value);
    LOG("set: %d\n", res);

    if (!res) {
      snprintf(msg, BUFFER_LENGTH, "SUCCESS");
    } else {
      snprintf(msg, BUFFER_LENGTH, "FAILURE");
    }
    break;
  }

  case KVSTORE_CMD_GET: {

    char *value = kvstore_array_get(key);
    if (value != NULL) {
      snprintf(msg, BUFFER_LENGTH, "%s", value);
    } else {
      snprintf(msg, BUFFER_LENGTH, "NOT EXIST");
    }
    break;
    LOG("get: %s\n", value);
  }
  case KVSTORE_CMD_DEL: {
    LOG("del\n");

    int res = kvstore_array_delete(key);
    if (res < 0) {
      snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
    } else if (res == 0) {
      snprintf(msg, BUFFER_LENGTH, "%s", "SUCCESS");
    } else {
      snprintf(msg, BUFFER_LENGTH, "%s", "NOT EXIST");
    }
    break;
  }
  case KVSTORE_CMD_MOD: {
    LOG("mod\n");
    int res = kvstore_array_modify(key, value);
    if (res < 0) {
      snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
    } else if (res == 0) {
      snprintf(msg, BUFFER_LENGTH, "%s", "SUCCESS");
    } else {
      snprintf(msg, BUFFER_LENGTH, "%s", "NOT EXIST");
    }

    break;
  }
  default:
    LOG("cmd: %s\n", tokens[0]);
    assert(0);
  }
}

// SET key value
int kvstore_request(connection_t *item) {
  LOG("kvstore_request called for fd: %d\n", item->fd);
  LOG("recv: %s\n", item->rbuffer);

  char *msg = item->rbuffer;
  char *tokens[KVSTORE_MAX_TOKEN];

  int token_count = kvstore_split_token(msg, tokens);
  int idx = 0;
  for (idx = 0; idx < token_count; idx++) {
    LOG("idx: %d, token: %s\n", idx, tokens[idx]);
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
