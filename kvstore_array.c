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
  array_table[array_idx].key = kcopy;
  array_table[array_idx].value = vcopy;
  array_idx++;
  return 0;
}

char *kvstore_array_get(char *key) {
  if (key == NULL) {
    return NULL;
  }
  int idx = 0;
  for (idx = 0; idx < array_idx; idx++) {
    if (strcmp(array_table[idx].key, key) == 0) {
      return array_table[idx].value;
    }
  }
  return NULL;
}
