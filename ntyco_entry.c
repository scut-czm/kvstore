
#include "kvstore.h"
#include <nty_coroutine.h>

#include <arpa/inet.h>

#define MAX_CLIENT_NUM 1000000
#define TIME_SUB_MS(tv1, tv2)                                                  \
  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)

void server_reader(void *arg) {
  int fd = *(int *)arg;
  free(arg);
  int ret = 0;

  struct pollfd fds;
  fds.fd = fd;
  fds.events = POLLIN;

  while (1) {
#if 0
    char buf[1024] = {0};
    ret = nty_recv(fd, buf, 1024, 0);
    if (ret > 0) {
      if (fd > MAX_CLIENT_NUM)
        printf("read from server: %.*s\n", ret, buf);

      ret = nty_send(fd, buf, strlen(buf), 0);
      if (ret == -1) {
        nty_close(fd);
        break;
      }
    } else if (ret == 0) {
      nty_close(fd);
      break;
    }
    }
#endif

    connection_t item = {0};
    ret = nty_recv(fd, item.rbuffer, BUFFER_LENGTH, 0);
    if (ret > 0) {
      if (fd > MAX_CLIENT_NUM)
        printf("read from server: %.*s\n", ret, item.rbuffer);

      kvstore_request(&item);
      item.wlen = strlen(item.wbuffer);
      ret = nty_send(fd, item.wbuffer, item.wlen, 0);
      if (ret == -1) {
        nty_close(fd);
        break;
      }
    } else if (ret == 0) {
      nty_close(fd);
      break;
    } else if (ret < 0) {

      nty_close(fd);
      break;
    }
  }
}

void server(void *arg) {
#if 0
  unsigned short port = *(unsigned short *)arg;
  free(arg);

  int fd = nty_socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0)
    return;

  struct sockaddr_in local, remote;
  local.sin_family = AF_INET;
  local.sin_port = htons(port);
  local.sin_addr.s_addr = INADDR_ANY;
  bind(fd, (struct sockaddr *)&local, sizeof(struct sockaddr_in));

  listen(fd, 20);
  printf("listen port : %d\n", port);

  struct timeval tv_begin;
  gettimeofday(&tv_begin, NULL);

  while (1) {

    socklen_t len = sizeof(struct sockaddr_in);
    int cli_fd = nty_accept(fd, (struct sockaddr *)&remote, &len);
    if (cli_fd % 1000 == 999) {

      struct timeval tv_cur;
      memcpy(&tv_cur, &tv_begin, sizeof(struct timeval));

      gettimeofday(&tv_begin, NULL);
      int time_used = TIME_SUB_MS(tv_begin, tv_cur);

      printf("client fd : %d, time_used: %d\n", cli_fd, time_used);
    }
    printf("new client comming\n");

    nty_coroutine *read_co;
    int *arg = (int *)malloc(sizeof(int));
    *arg = cli_fd;
    nty_coroutine_create(&read_co, server_reader, arg);
  }
#endif

  unsigned short port = *(unsigned short *)arg;
  free(arg);

  int fd = nty_socket(AF_INET, SOCK_STREAM, 0);
  if (fd < 0) {
    printf("socket creation failed\n");
    return;
  }

  // 设置 SO_REUSEADDR 选项
  int opt = 1;
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    printf("setsockopt failed\n");
    nty_close(fd);
    return;
  }

  struct sockaddr_in local, remote;
  local.sin_family = AF_INET;
  local.sin_port = htons(port);
  local.sin_addr.s_addr = INADDR_ANY;

  // 检查 bind 返回值
  if (bind(fd, (struct sockaddr *)&local, sizeof(struct sockaddr_in)) < 0) {
    printf("bind failed on port %d: %s\n", port, strerror(errno));
    nty_close(fd);
    return;
  }

  // 检查 listen 返回值
  if (listen(fd, 20) < 0) {
    printf("listen failed on port %d: %s\n", port, strerror(errno));
    nty_close(fd);
    return;
  }

  printf("Successfully listening on port : %d\n", port);

  struct timeval tv_begin;
  gettimeofday(&tv_begin, NULL);

  while (1) {
    socklen_t len = sizeof(struct sockaddr_in);
    int cli_fd = nty_accept(fd, (struct sockaddr *)&remote, &len);

    if (cli_fd < 0) {
      printf("accept failed: %s\n", strerror(errno));
      continue;
    }

    if (cli_fd % 1000 == 999) {
      struct timeval tv_cur;
      memcpy(&tv_cur, &tv_begin, sizeof(struct timeval));
      gettimeofday(&tv_begin, NULL);
      int time_used = TIME_SUB_MS(tv_begin, tv_cur);
      printf("client fd : %d, time_used: %d\n", cli_fd, time_used);
    }
    printf("new client coming from %s:%d\n", inet_ntoa(remote.sin_addr),
           ntohs(remote.sin_port));

    nty_coroutine *read_co;
    int *arg = (int *)malloc(sizeof(int));
    *arg = cli_fd;
    nty_coroutine_create(&read_co, server_reader, arg);
  }
}

int ntyco_entry() {
  nty_coroutine *co = NULL;

  int i = 0;
  unsigned short base_port = 2048;
  for (i = 0; i < 1; i++) {
    unsigned short *port = (unsigned short *)calloc(1, sizeof(unsigned short));
    *port = base_port + i;
    nty_coroutine_create(&co, server, port); ////////no run
  }

  nty_schedule_run(); // run

  return 0;
}
