#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define RED 1
#define BLACK 2

/**********************定义红黑树 start***************************/
typedef int KEY_TYPE;

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
} rbtree;

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
    if (z->key < x->key)
      x = x->node.left;
    else if (z->key > x->key)
      x = x->node.right;
    else
      return;
  }

  z->node.parent = y;
  if (y == T->nil)
    T->root = z;
  else {
    if (y->key > z->key)
      y->node.left = z;
    else
      y->node.right = z;
  }

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
    z->key = y->key;
    z->value = y->value;
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
    if (key < node->key) {
      node = node->node.left;
    } else if (key > node->key) {
      node = node->node.right;
    } else {
      return node;
    }
  }
  return T->nil;
}
/**********************红黑树查找 end***************************/

/**********************红黑树使用示例 start***************************/
// 遍历
void rbtree_traversal(rbtree *T, rbtree_node *node) {
  if (node != T->nil) {
    rbtree_traversal(T, node->node.left);
    printf("key:%d, color:%d\n", node->key, node->node.color);
    rbtree_traversal(T, node->node.right);
  }
}

#if 0
int main() {

  int keyArray[20] = {24, 25, 13, 35, 23, 26, 67, 47, 38, 98,
                      20, 19, 17, 49, 12, 21, 9,  18, 14, 15};

  rbtree *T = (rbtree *)malloc(sizeof(rbtree));
  if (T == NULL) {
    printf("malloc failed\n");
    return -1;
  }

  T->nil = (rbtree_node *)malloc(sizeof(rbtree_node));
  T->nil->node.color = BLACK;
  T->root = T->nil;

  rbtree_node *node = T->nil;
  int i = 0;
  for (i = 0; i < 20; i++) {
    node = (rbtree_node *)malloc(sizeof(rbtree_node));
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
    free(cur);
    printf("show rbtree: \n");
    rbtree_traversal(T, T->root);
    printf("----------------------------------------\n");
  }
}
#endif

/**********************红黑树使用示例 end***************************/
