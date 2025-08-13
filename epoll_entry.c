#include "kvstore.h"

int accept_cb(int fd);

int recv_cb(int fd);

int send_cb(int fd);

int epfd = 0;

struct conn_item connlist[1048576] = {0}; // 1024修改，链接数

int set_event(int fd, int event, int flag) {
  if (flag) {
    // 1 add, 0 mod;
    struct epoll_event ev;
    ev.events = event;
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
  } else {
    struct epoll_event ev;
    ev.events = event;
    ev.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
  }
  return 0;
}
int accept_cb(int fd) {

  struct sockaddr_in clientaddr;
  socklen_t len = sizeof(clientaddr);

  int clientfd = accept(fd, (struct sockaddr *)&clientaddr, &len);

  /*printf("clientfd=%d,fd=%d\n", clientfd, fd);*/
  set_event(clientfd, EPOLLIN, 1);

  // 初始化conn_item结构体
  connlist[clientfd].fd = clientfd;
  memset(connlist[clientfd].rbuffer, 0, BUFFER_LENGTH);
  connlist[clientfd].rlen = 0;
  memset(connlist[clientfd].wbuffer, 0, BUFFER_LENGTH);
  connlist[clientfd].wlen = 0;
  connlist[clientfd].send_callback = send_cb;
  connlist[clientfd].recv_t.recv_callback = recv_cb;
  // 返回clientfd
  return clientfd;
}

int recv_cb(int fd) {
  char *buf = connlist[fd].rbuffer;
  memset(buf, 0, BUFFER_LENGTH);
  // int idx = connlist[fd].rlen;
  // int length = BUFFER_LENGTH - idx;
  int count = recv(fd, buf, BUFFER_LENGTH, 0);

  if (count == 0) {
    printf("disconnect\n");
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    return -1;
  }
  connlist[fd].rlen = count;
  // /**/
  // /*send(fd, buf, connlist[fd].idx, 0);*/
  // printf("clientfd: %d, count: %d,buf: %s\n", fd, count, buf);
  // // 设置写事件，发送buf
  //
  // // need to send
  // memcpy(connlist[fd].wbuffer, connlist[fd].rbuffer, connlist[fd].rlen);
  // connlist[fd].wlen = connlist[fd].rlen;
  // // 使得收什么发什么,重置rlen，使得发什么收什么
  // connlist[fd].rlen -= connlist[fd].rlen;

  kvstore_request(&connlist[fd]);
  connlist[fd].wlen = strlen(connlist[fd].wbuffer);
  // 将事件设置为写事件，实现send发送
  set_event(fd, EPOLLOUT, 0);

  return count;
}

int send_cb(int fd) {
  char *buf = connlist[fd].wbuffer;
  int idx = connlist[fd].wlen;

  int count = send(fd, buf, idx, 0);
  // 发送完后将事件设置为读事件，再次recv
  set_event(fd, EPOLLIN, 0);

  return count;
}

int epoll_entry() {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  // sockfd=3
  struct sockaddr_in serveraddr;
  memset(&serveraddr, 0, sizeof(struct sockaddr_in));

  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons(2048);

  int ret =
      bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(struct sockaddr));
  if (ret == -1) {
    perror("bind");
    return -1;
  }

  listen(sockfd, 10);
  // 以上服务器基本流程
  //----------------------------------------------------------------------------------------------
  // 设置sockfd=3的回调函数为accept，只要有连接就会accept
  connlist[sockfd].fd = sockfd;

  connlist[sockfd].recv_t.accept_callback = accept_cb;

  epfd = epoll_create(1);
  /*printf("epfd=%d\n", epfd); epfd=4*/

  set_event(sockfd, EPOLLIN, 1);

  struct epoll_event events[1024] = {0};

  while (1) {                                        // mainloop
    int nready = epoll_wait(epfd, events, 1024, -1); // nready=1

    int i = 0;

    for (i = 0; i < nready; ++i) {

      int connfd = events[i].data.fd;
      /*if (connfd == sockfd) {*/
      /*  int clientfd = connlist[connfd].recv_t.accept_callback(connfd);*/
      /*  printf("clientfd=%d\n", clientfd);*/
      if (events[i].events & EPOLLIN) { // 有accept和recv两种触发手段
        // 读事件

        int count = connlist[connfd].recv_t.recv_callback(connfd);

        /*if (count > 0 &&*/
        /*    connfd != 3) { // 不是sockfd为3的accept事件，是recv触发的*/
        /*  printf("recv count: %d <-----buffer: %s\n", count,*/
        /*         connlist[connfd].rbuffer);*/
        /*}*/

      } else if (events[i].events & EPOLLOUT) { // 触发写事件
        // 写事件

        int count = connlist[connfd].send_callback(connfd);

        /*if (count != 0) {*/
        /*  printf("send----->buffer: %s\n", connlist[connfd].wbuffer);*/
        /*}*/
      }
    }
  }

  close(sockfd);
  return 0;
}
