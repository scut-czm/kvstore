#include "kvstore.h"

#define RED 1
#define BLACK 2
#define ENABLE_KEY_CHAR 1
#define MAX_KEY_LEN 256
#define MAX_VALUE_LEN 1024

/**********************定义红黑树 start***************************/
#if ENABLE_KEY_CHAR
typedef char *KEY_TYPE;
#else
typedef int KEY_TYPE;
#endif

// 红黑树模板，提高复用性
#define RBTREE_ENTRT(name, type)                                               \
  struct name {                                                                \
    struct type *right;                                                        \
    struct type *left;                                                         \
    struct type *parent;                                                       \
    unsigned char color;                                                       \
  }

typedef struct _rbtree_node {
  KEY_TYPE key;
  void *value;
#if 0
	struct _rbtree_node *right;
	struct _rbtree_node *left;
	struct _rbtree_node *parent;

	unsigned char color;
#else
  RBTREE_ENTRT(, _rbtree_node) node;
  // RBTREE_ENTRT(, _rbtree_node) node2;
#endif
} rbtree_node;

typedef struct _rbtree {
  rbtree_node *root;
  rbtree_node *nil;
  int count;
} rbtree;

// 全局rbtree
rbtree Tree = {
    .root = NULL,
    .nil = NULL,
    .count = 0,
};
/**********************定义红黑树 end***************************/

/**********************红黑树左旋 start***************************/
void rbtree_left_rotate(rbtree *T, rbtree_node *x) {
  rbtree_node *y = x->node.right;
  // 1
  x->node.right = y->node.left;
  if (y->node.left != T->nil) {
    y->node.left->node.parent = x;
  }
  // 2
  y->node.parent = x->node.parent;
  if (x->node.parent == T->nil)
    T->root = y;
  else if (x == x->node.parent->node.left)
    x->node.parent->node.left = y;
  else
    x->node.parent->node.right = y;
  // 3
  y->node.left = x;
  x->node.parent = y;
}
/**********************红黑树左旋 end***************************/

/**********************红黑树右旋 start***************************/
/*
 * x改为y，y改为x，右改为左，左改为右
 */
void rbtree_right_rotate(rbtree *T, rbtree_node *y) {
  rbtree_node *x = y->node.left;
  // 1
  y->node.left = x->node.right;
  if (x->node.right != T->nil) {
    x->node.right->node.parent = y;
  }
  // 2
  x->node.parent = y->node.parent;
  if (y->node.parent == T->nil)
    T->root = x;
  else if (y == y->node.parent->node.right)
    y->node.parent->node.right = x;
  else
    y->node.parent->node.left = x;
  // 3
  x->node.right = y;
  y->node.parent = x;
}
/**********************红黑树右旋 end***************************/

/**********************红黑树插入 start***************************/
// 调整
void rbtree_insert_fixup(rbtree *T, rbtree_node *z) {
  // 红黑树特性之一：如果一个结点是红的，则它的两个儿子是黑的
  while (z->node.parent->node.color == RED) {
    if (z->node.parent == z->node.parent->node.parent->node.left) {
      rbtree_node *y = z->node.parent->node.parent->node.right;
      if (y->node.color == RED) // 叔父结点为红色
      {
        z->node.parent->node.color = BLACK;
        y->node.color = BLACK;
        z->node.parent->node.parent->node.color = RED;

        // 保证 Z 永远是红色，才能调整
        z = z->node.parent->node.parent;
      } else // y==black
      {
        if (z == z->node.parent->node.right) {
          z = z->node.parent;
          rbtree_left_rotate(T, z);
        }
        z->node.parent->node.color = BLACK;
        z->node.parent->node.parent->node.color = RED;
        // 祖父结点旋转
        rbtree_right_rotate(T, z->node.parent->node.parent);
      }
    } else {
      rbtree_node *y = z->node.parent->node.parent->node.left;
      if (y->node.color == RED) // 叔父结点为红色
      {
        z->node.parent->node.color = BLACK;
        y->node.color = BLACK;
        z->node.parent->node.parent->node.color = RED;

        // 保证 Z 永远是红色，才能调整
        z = z->node.parent->node.parent;
      } else {
        if (z == z->node.parent->node.left) {
          z = z->node.parent;
          rbtree_right_rotate(T, z);
        }

        z->node.parent->node.color = BLACK;
        z->node.parent->node.parent->node.color = RED;
        rbtree_left_rotate(T, z->node.parent->node.parent);
      }
    }
  }
  T->root->node.color = BLACK;
}
// 插入到底部
void rbtree_insert(rbtree *T, rbtree_node *z) {
  rbtree_node *y = T->nil;
  rbtree_node *x = T->root;
  while (x != T->nil) {
    y = x;
#if ENABLE_KEY_CHAR
    if (strcmp(z->key, x->key) < 0) {
      x = x->node.left;
    } else if (strcmp(z->key, x->key) > 0) {
      x = x->node.right;
    } else {
      return; // key已存在
    }
#else
    if (z->key < x->key)
      x = x->node.left;
    else if (z->key > x->key)
      x = x->node.right;
    else
      return;
#endif
  }

  z->node.parent = y;
  if (y == T->nil)
    T->root = z;
#if ENABLE_KEY_CHAR
  else if (strcmp(z->key, y->key) < 0)
#else
  else if (y->key > z->key)
#endif

    y->node.left = z;
  else
    y->node.right = z;

  z->node.left = z->node.right = T->nil;
  z->node.color = RED;
  rbtree_insert_fixup(T, z);
}

/**********************红黑树插入 end***************************/

/**********************红黑树删除 start***************************/
rbtree_node *rbtree_mini(rbtree *T, rbtree_node *x) {
  while (x->node.left != T->nil) {
    x = x->node.left;
  }
  return x;
}

rbtree_node *rbtree_maxi(rbtree *T, rbtree_node *x) {
  while (x->node.right != T->nil) {
    x = x->node.right;
  }
  return x;
}

rbtree_node *rbtree_successor(rbtree *T, rbtree_node *x) {
  rbtree_node *y = x->node.parent;
  if (x->node.right != T->nil) {
    return rbtree_mini(T, x->node.right);
  }

  while ((y != T->nil) && (x == y->node.right)) {
    x = y;
    y = y->node.parent;
  }
  return y;
}
// 调整
void rbtree_delete_fixup(rbtree *T, rbtree_node *x) {

  while ((x != T->root) && (x->node.color == BLACK)) {
    if (x == x->node.parent->node.left) {

      rbtree_node *w = x->node.parent->node.right;
      if (w->node.color == RED) {
        w->node.color = BLACK;
        x->node.parent->node.color = RED;

        rbtree_left_rotate(T, x->node.parent);
        w = x->node.parent->node.right;
      }

      if ((w->node.left->node.color == BLACK) &&
          (w->node.right->node.color == BLACK)) {
        w->node.color = RED;
        x = x->node.parent;
      } else {

        if (w->node.right->node.color == BLACK) {
          w->node.left->node.color = BLACK;
          w->node.color = RED;
          rbtree_right_rotate(T, w);
          w = x->node.parent->node.right;
        }

        w->node.color = x->node.parent->node.color;
        x->node.parent->node.color = BLACK;
        w->node.right->node.color = BLACK;
        rbtree_left_rotate(T, x->node.parent);

        x = T->root;
      }

    } else {

      rbtree_node *w = x->node.parent->node.left;
      if (w->node.color == RED) {
        w->node.color = BLACK;
        x->node.parent->node.color = RED;
        rbtree_right_rotate(T, x->node.parent);
        w = x->node.parent->node.left;
      }

      if ((w->node.left->node.color == BLACK) &&
          (w->node.right->node.color == BLACK)) {
        w->node.color = RED;
        x = x->node.parent;
      } else {

        if (w->node.left->node.color == BLACK) {
          w->node.right->node.color = BLACK;
          w->node.color = RED;
          rbtree_left_rotate(T, w);
          w = x->node.parent->node.left;
        }

        w->node.color = x->node.parent->node.color;
        x->node.parent->node.color = BLACK;
        w->node.left->node.color = BLACK;
        rbtree_right_rotate(T, x->node.parent);

        x = T->root;
      }
    }
  }

  x->node.color = BLACK;
}

rbtree_node *rbtree_delete(rbtree *T, rbtree_node *z) {
  rbtree_node *y = T->nil;
  rbtree_node *x = T->nil;

  if ((z->node.left == T->nil) || (z->node.right == T->nil)) {
    y = z;
  } else {
    y = rbtree_successor(T, z);
  }

  if (y->node.left != T->nil)
    x = y->node.left;
  else if (y->node.right != T->nil)
    x = y->node.right;

  x->node.parent = y->node.parent;
  if (y->node.parent == T->nil)
    T->root = x;
  else if (y == y->node.parent->node.left)
    y->node.parent->node.left = x;
  else
    y->node.parent->node.right = x;

  if (y != z) {
#if ENABLE_KEY_CHAR
    void *temp = z->key;
    z->key = y->key;
    y->key = (char *)temp;

    temp = z->value;
    z->value = y->value;
    y->value = temp;
#else
    z->key = y->key;
    z->value = y->value;
#endif
  }
  // 调整
  if (y->node.color == BLACK) {
    rbtree_delete_fixup(T, x);
  }

  return y;
}

/**********************红黑树删除 end***************************/

/**********************红黑树查找 start***************************/
rbtree_node *rbtree_search(rbtree *T, KEY_TYPE key) {

  rbtree_node *node = T->root;
  while (node != T->nil) {
#if ENABLE_KEY_CHAR
    if (strcmp(key, node->key) < 0) {
      node = node->node.left;
    } else if (strcmp(key, node->key) > 0) {
      node = node->node.right;
    } else {
      return node;
    }
#else
    if (key < node->key) {
      node = node->node.left;
    } else if (key > node->key) {
      node = node->node.right;
    } else {
      return node;
    }
  }
#endif
  }
  return T->nil;
}
/**********************红黑树查找 end***************************/

/**********************红黑树使用示例 start***************************/
// 遍历
void rbtree_traversal(rbtree *T, rbtree_node *node) {
  if (node != T->nil) {
    rbtree_traversal(T, node->node.left);
#if ENABLE_KEY_CHAR
    printf("key:%s, color:%d\n", node->key, node->node.color);
#else
    printf("key:%d, color:%d\n", node->key, node->node.color);
#endif
    rbtree_traversal(T, node->node.right);
  }
}

/*********************接口层封装*****************************/

int rbtree_create(rbtree *tree) {
  if (tree == NULL) {
    return -1;
  }
  tree->nil = (rbtree_node *)kvstore_malloc(sizeof(rbtree_node));
  if (tree->nil == NULL) {
    return -1;
  }
  tree->nil->key = (char *)kvstore_malloc(1);
  if (tree->nil->key == NULL) {
    kvstore_free(tree->nil);
    return -1;
  }
  *(tree->nil->key) = '\0';
  tree->nil->node.color = BLACK;
  tree->nil->node.left = tree->nil->node.right = tree->nil->node.parent =
      tree->nil;
  tree->root = tree->nil;
  tree->count = 0;
}

void rbtree_destroy(rbtree *tree) {
  if (tree == NULL) {
    return;
  }
  if (tree->nil->key) {
    kvstore_free(tree->nil->key);
  }

  rbtree_node *node = tree->root;
  while (node != tree->nil) {
    node = rbtree_mini(tree, tree->root);
    if (node == tree->nil) {
      break;
    }
    node = rbtree_delete(tree, node);
    if (node != NULL) {
      if (node->key) {
        kvstore_free(node->key);
      }
      if (node->value) {
        kvstore_free(node->value);
      }
      kvstore_free(node);
    }
  }
}

int rbtree_set(rbtree *tree, char *key, char *value) {
  rbtree_node *node = (rbtree_node *)kvstore_malloc(sizeof(rbtree_node));
  if (node == NULL) {
    return -1;
  }
  node->key = (char *)kvstore_malloc(strlen(key) + 1);
  if (node->key == NULL) {
    kvstore_free(node);
    return -1;
  }
  memset(node->key, 0, strlen(key) + 1);
  strcpy(node->key, key);

  node->value = (char *)kvstore_malloc(strlen(value) + 1);
  if (node->value == NULL) {
    kvstore_free(node->key);
    kvstore_free(node);
    return -1;
  }
  memset(node->value, 0, strlen(value) + 1);
  strcpy((char *)node->value, value);

  rbtree_insert(tree, node);
  tree->count++;

  return 0;
}

char *rbtree_get(rbtree *tree, char *key) {
  rbtree_node *node = rbtree_search(tree, key);
  if (node == tree->nil) {
    return NULL; // key不存在
  }
  return (char *)node->value;
}

int rbtree_del(rbtree *tree, char *key) {
  rbtree_node *node = rbtree_search(tree, key);

  if (node == tree->nil) {
    return -1; // key不存在
  }
  rbtree_node *cur = rbtree_delete(tree, node);
  if (cur != NULL) {

    if (cur->key) {
      kvstore_free(cur->key);
    }
    if (cur->value) {
      kvstore_free(cur->value);
    }
    kvstore_free(cur);
    tree->count--;
  }
  return 0;
}

int rbtree_modify(rbtree *tree, char *key, char *value) {
  rbtree_node *node = rbtree_search(tree, key);
  if (node == tree->nil) {
    return -1; // key不存在
  }

  void *tmp = node->value;
  kvstore_free(tmp);

  node->value = (char *)kvstore_malloc(strlen(value) + 1);
  if (node->value == NULL) {
    return -1;
  }
  strcpy((char *)node->value, value);

  return 0;
}

/********************** 红黑树适配层 ***************************/

int kvstore_rbtree_create(rbtree *tree) { return rbtree_create(&Tree); }

void kvstore_rbtree_destroy(rbtree *tree) { rbtree_destroy(&Tree); }

int kvstore_rbtree_set(char *key, char *value) {
  return rbtree_set(&Tree, key, value);
}

int kvstore_rbtree_modify(char *key, char *value) {
  return rbtree_modify(&Tree, key, value);
}

int kvstore_rbtree_delete(char *key) { return rbtree_del(&Tree, key); }

char *kvstore_rbtree_get(char *key) { return rbtree_get(&Tree, key); }

int kvstore_rbtree_count(void) { return Tree.count; }

#if 0
int main() {
  rbtree *tree = (rbtree *)kvstore_malloc(sizeof(rbtree));
  rbtree_create(tree);

  char *key = "name";
  char *value = "zhangsan";

  int res = rbtree_set(tree, key, value);

  char *get_value = rbtree_get(tree, key);
  if (get_value != NULL) {
    printf("Get value: %s\n", get_value);
  } else {
    printf("Key not found.\n");
  }
  rbtree_modify(tree, key, "lisi");
  get_value = rbtree_get(tree, key);
  if (get_value != NULL) {
    printf("Get value after modify: %s\n", get_value);
  } else {
    printf("Key not found after modify.\n");
  }
  rbtree_del(tree, key);
  get_value = rbtree_get(tree, key);
  if (get_value != NULL) {
    printf("Get value after delete: %s\n", get_value);
  } else {
    printf("Key not found after delete.\n");
  }
  rbtree_destroy(tree);
}
#endif

#if 0
int main() {

  int keyArray[20] = {24, 25, 13, 35, 23, 26, 67, 47, 38, 98,
                      20, 19, 17, 49, 12, 21, 9,  18, 14, 15};

  rbkvstore_free *T = (rbkvstore_free *)mkvstore_malloc(sizeof(rbkvstore_free));
  if (T == NULL) {
    printf("mkvstore_malloc failed\n");
    return -1;
  }

  T->nil = (rbtree_node *)mkvstore_malloc(sizeof(rbtree_node));
  T->nil->node.color = BLACK;
  T->root = T->nil;

  rbtree_node *node = T->nil;
  int i = 0;
  for (i = 0; i < 20; i++) {
    node = (rbtree_node *)mkvstore_malloc(sizeof(rbtree_node));
    node->key = keyArray[i];
    node->value = NULL;

    rbtree_insert(T, node);
  }

  rbtree_traversal(T, T->root);
  printf("----------------------------------------\n");

  for (i = 0; i < 20; i++) {
    printf("search key = %d\n", keyArray[i]);
    rbtree_node *node = rbtree_search(T, keyArray[i]);
    printf("delete key = %d\n", node->key);
    rbtree_node *cur = rbtree_delete(T, node);
    kvstore_free(cur);
    printf("show rbkvstore_free: \n");
    rbtree_traversal(T, T->root);
    printf("----------------------------------------\n");
  }
}
#endif

/**********************红黑树使用示例 end***************************/
