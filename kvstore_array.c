#include "kvstore.h"

array_t array;

// 接口层
int array_set(array_t *arr, char *key, char *value) {
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

  if (arr->array_idx >= KVS_ARRAY_SIZE) {
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
  for (i = 0; i < arr->array_idx; i++) {
    if (arr->array_table[i].key == NULL) {
      arr->array_table[i].key = kcopy;
      arr->array_table[i].value = vcopy;
      arr->array_idx++;
      return 0; // 成功插入
    }
  }
  if (i <= KVS_ARRAY_SIZE && i == arr->array_idx) {
    // 如果没有空位，则插入到最后
    arr->array_table[arr->array_idx].key = kcopy;
    arr->array_table[arr->array_idx].value = vcopy;
    arr->array_idx++;
  } else {
    kvstore_free(kcopy);
    kvstore_free(vcopy);
    return -1; // 插入失败
  }
  return 0;
}

char *array_get(array_t *arr, char *key) {
  if (key == NULL) {
    return NULL;
  }
  int idx = 0;
  for (idx = 0; idx < arr->array_idx; idx++) {
    if (arr->array_table[idx].key == NULL) {
      return NULL; // 如果key为NULL，直接返回NULL
    }
    if (strcmp(arr->array_table[idx].key, key) == 0) {
      return arr->array_table[idx].value;
    }
  }
  return NULL;
}

int array_delete(array_t *arr, char *key) {
  if (key == NULL) {
    return -1;
  }
  int i = 0;
  for (i = 0; i < arr->array_idx; i++) {
    if (strcmp(arr->array_table[i].key, key) == 0) {
      kvstore_free(arr->array_table[i].value);
      arr->array_table[i].value = NULL;

      kvstore_free(arr->array_table[i].key);
      arr->array_table[i].key = NULL;

      arr->array_idx--;
      return 0; // 删除成功
    }
  }
  return i; // 返回未找到的索引
}

int array_modify(array_t *arr, char *key, char *value) {
  if (key == NULL || value == NULL) {
    return -1;
  }
  int i = 0;
  for (i = 0; i < arr->array_idx; i++) {
    if (strcmp(arr->array_table[i].key, key) == 0) {
      kvstore_free(arr->array_table[i].value);
      arr->array_table[i].value = NULL;

      char *vcopy = (char *)kvstore_malloc(strlen(value) + 1);
      strncpy(vcopy, value, strlen(value) + 1);

      arr->array_table[i].value = vcopy;
      return 0; // 修改成功
    }
  }
  return i;
}

int array_count(array_t *arr) { return arr->array_idx; }

/************************* 适配层 ***********************/

int kvstore_array_create(array_t *arr) {
  if (arr == NULL) {
    return -1;
  }
  arr->array_table = (kvs_array_item_t *)kvstore_malloc(
      KVS_ARRAY_SIZE * sizeof(kvs_array_item_t));
  if (arr->array_table == NULL) {
    return -1; // 分配内存失败
  }

  memset(arr->array_table, 0, KVS_ARRAY_SIZE * sizeof(kvs_array_item_t));

  arr->array_idx = 0;

  return 0;
}

void kvstore_array_destroy(array_t *arr) {
  if (arr == NULL) {
    return;
  }
  int i = 0;
  for (i = 0; i < arr->array_idx; i++) {
    kvstore_free(arr->array_table[i].key);
    kvstore_free(arr->array_table[i].value);
  }
  kvstore_free(arr->array_table);
  arr->array_table = NULL;
  arr->array_idx = 0;
  return;
}

int kvstore_array_set(char *key, char *value) {
  return array_set(&array, key, value);
}
char *kvstore_array_get(char *key) { return array_get(&array, key); }
int kvstore_array_delete(char *key) { return array_delete(&array, key); }
int kvstore_array_modify(char *key, char *value) {
  return array_modify(&array, key, value);
}
int kvstore_array_count(void) { return array_count(&array); }
