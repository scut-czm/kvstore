#include "kvstore.h"
kvs_array_item_t array_table[KVS_ARRAY_SIZE] = {0};

int array_idx = 0;

// __attribute__((constructor)) void kvstore_array_init() {
//   array_table = (kvs_array_item_t *)kvstore_malloc(sizeof(kvs_array_item_t) *
//                                                    KVS_ARRAY_SIZE);
//   if (array_table == NULL) {
//     fprintf(stderr, "Failed to allocate memory for array table\n");
//     exit(EXIT_FAILURE);
//   }
//   memset(array_table, 0, sizeof(kvs_array_item_t) * KVS_ARRAY_SIZE);
// }

int kvstore_array_set(char *key, char *value) {
  if (key == NULL || value == NULL) {
    return -1;
  }

  char *kcopy = (char *)kvstore_malloc(strlen(key) + 1);
  if (kcopy == NULL) {
    return -1;
  }
  strncpy(kcopy, key, strlen(key) + 1);

  char *vcopy = (char *)kvstore_malloc(strlen(value) + 1);
  if (vcopy == NULL) {
    kvstore_free(kcopy);
    return -1;
  }
  strncpy(vcopy, value, strlen(value) + 1);

  if (array_idx >= KVS_ARRAY_SIZE) {
    kvstore_free(kcopy);
    kvstore_free(vcopy);
    return -1; // Array is full
  }
#if 0
  array_table[array_idx].key = kcopy;
  array_table[array_idx].value = vcopy;
  array_idx++;
#endif
  // 判断前面是否有空位
  int i = 0;
  for (i = 0; i < array_idx; i++) {
    if (array_table[i].key == NULL) {
      array_table[i].key = kcopy;
      array_table[i].value = vcopy;
      array_idx++;
      return 0; // 成功插入
    }
  }
  if (i <= KVS_ARRAY_SIZE && i == array_idx) {
    // 如果没有空位，则插入到最后
    array_table[array_idx].key = kcopy;
    array_table[array_idx].value = vcopy;
    array_idx++;
  } else {
    kvstore_free(kcopy);
    kvstore_free(vcopy);
    return -1; // 插入失败
  }
  return 0;
}

char *kvstore_array_get(char *key) {
  if (key == NULL) {
    return NULL;
  }
  int idx = 0;
  for (idx = 0; idx < array_idx; idx++) {
    if (array_table[idx].key == NULL) {
      return NULL; // 如果key为NULL，直接返回NULL
    }
    if (strcmp(array_table[idx].key, key) == 0) {
      return array_table[idx].value;
    }
  }
  return NULL;
}

int kvstore_array_delete(char *key) {
  if (key == NULL) {
    return -1;
  }
  int i = 0;
  for (i = 0; i < array_idx; i++) {
    if (strcmp(array_table[i].key, key) == 0) {
      kvstore_free(array_table[i].value);
      array_table[i].value = NULL;

      kvstore_free(array_table[i].key);
      array_table[i].key = NULL;

      array_idx--;
      return 0; // 删除成功
    }
  }
  return i; // 返回未找到的索引
}

int kvstore_array_modify(char *key, char *value) {
  if (key == NULL || value == NULL) {
    return -1;
  }
  int i = 0;
  for (i = 0; i < array_idx; i++) {
    if (strcmp(array_table[i].key, key) == 0) {
      kvstore_free(array_table[i].value);
      array_table[i].value = NULL;

      char *vcopy = (char *)kvstore_malloc(strlen(value) + 1);
      strncpy(vcopy, value, strlen(value) + 1);

      array_table[i].value = vcopy;
      return 0; // 修改成功
    }
  }
  return i;
}
