#include "kvstore.h"

#define KVSTORE_MAX_TOKEN 128

const char *commands[] = {

    "SET",  "GET",  "DEL",  "MOD",  "COUNT",
    "RSET", "RGET", "RDEL", "RMOD", "RCOUNT",
};

enum {
  KVSTORE_CMD_START = 0,
  KVSTORE_CMD_SET = KVSTORE_CMD_START,
  KVSTORE_CMD_GET,
  KVSTORE_CMD_DEL,
  KVSTORE_CMD_MOD,
  KVSTORE_CMD_COUNT,

  KVSTORE_CMD_RSET,
  KVSTORE_CMD_RGET,
  KVSTORE_CMD_RDEL,
  KVSTORE_CMD_RMOD,
  KVSTORE_CMD_RCOUNT,
  KVSTORE_CMD_SIZE,
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
  for (cmd = KVSTORE_CMD_START; cmd < KVSTORE_CMD_SIZE; cmd++) {
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
  // array kvstore
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
    LOG("get: %s\n", value);

    if (value != NULL) {
      snprintf(msg, BUFFER_LENGTH, "%s", value);
    } else {
      snprintf(msg, BUFFER_LENGTH, "NOT EXIST");
    }
    break;
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
  case KVSTORE_CMD_COUNT: {
    LOG("count\n");
    int count = kvstore_array_count();
    if (count > 0) {
      snprintf(msg, BUFFER_LENGTH, "%d", count);
    } else {
      snprintf(msg, BUFFER_LENGTH, "%s", "NULL");
    }
    break;
  }

    // rbtree kvstore
  case KVSTORE_CMD_RSET: {
    LOG("rset\n");
    int res = kvstore_rbtree_set(key, value);
    if (!res) {
      snprintf(msg, BUFFER_LENGTH, "SUCCESS");
    } else {
      snprintf(msg, BUFFER_LENGTH, "FAILURE");
    }
    break;
  }
  case KVSTORE_CMD_RGET: {

    char *value = kvstore_rbtree_get(key);
    LOG("rget: %s\n", value);

    if (value != NULL) {
      snprintf(msg, BUFFER_LENGTH, "%s", value);
    } else {
      snprintf(msg, BUFFER_LENGTH, "NOT EXIST");
    }
    break;
  }

  case KVSTORE_CMD_RDEL: {
    LOG("rdel\n");

    int res = kvstore_rbtree_delete(key);

    if (res < 0) {
      snprintf(msg, BUFFER_LENGTH, "%s", "NO EXIST");
    } else {
      snprintf(msg, BUFFER_LENGTH, "%s", "SUCCESS");
    }
    break;
  }

  case KVSTORE_CMD_RMOD: {
    LOG("rmod\n");
    int res = kvstore_rbtree_modify(key, value);
    if (res < 0) {
      snprintf(msg, BUFFER_LENGTH, "%s", "ERROR");
    } else if (res == 0) {
      snprintf(msg, BUFFER_LENGTH, "%s", "SUCCESS");
    } else {
      snprintf(msg, BUFFER_LENGTH, "%s", "NOT EXIST");
    }
    break;
  }
  case KVSTORE_CMD_RCOUNT: {
    LOG("rcount\n");
    int count = kvstore_rbtree_count();
    if (count > 0) {
      snprintf(msg, BUFFER_LENGTH, "%d", count);
    } else {
      snprintf(msg, BUFFER_LENGTH, "%d", 0);
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

int init_kvengine(void) {
#if ENABLE_ARRAY_KVENGINE
  kvstore_array_create(&array);
#endif

#if ENABLE_RBTREE_KVENGINE
  kvstore_rbtree_create(&Tree);
#endif
  return 0;
}

int exit_kvengine(void) {
#if ENABLE_ARRAY_KVENGINE
  kvstore_array_destroy(&array);
#endif

#if ENABLE_RBTREE_KVENGINE

  kvstore_rbtree_destroy(&Tree);
#endif
  return 0;
}

int main() {
  int res = init_kvengine();
  if (res < 0) {
    LOG("init kvstore failed\n");
    return -1;
  }
#if (ENABLE_NETWORK_SELECT == NEWORK_EPOLL)
  epoll_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_NTYCO)
  ntyco_entry();
#elif (ENABLE_NETWORK_SELECT == NETWORK_IOURING)
#endif

  exit_kvengine();
  return 0;
}
