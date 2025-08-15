
#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define TIME_SUB_MS(tv1, tv2)                                                  \
  ((tv1.tv_sec - tv2.tv_sec) * 1000 + (tv1.tv_usec - tv2.tv_usec) / 1000)

typedef struct test_context_s {
  char serverio[16];
  int port;
  int threadnum;
  int connection;
  int requestion;

  int failde;
} test_context_t;

// 客户端绑定---connection
int connect_tcpserver(const char *ip, unsigned short port) {
  int connfd = socket(AF_INET, SOCK_STREAM, 0);
  if (connfd < 0) {
    perror("socket");
    return -1;
  }
  struct sockaddr_in tcpserver_addr;

  memset(&tcpserver_addr, 0, sizeof(tcpserver_addr));

  tcpserver_addr.sin_family = AF_INET;
  tcpserver_addr.sin_port = htons(port);
  tcpserver_addr.sin_addr.s_addr = inet_addr(ip);

  int ret = connect(connfd, (struct sockaddr *)&tcpserver_addr,
                    sizeof(struct sockaddr_in));
  if (ret < 0) {
    perror("connect");
    close(connfd);
    return -1;
  }
  return connfd;
}

int send_msg(int connfd, char *msg, int length) {
  int res = send(connfd, msg, length, 0);
  if (res < 0) {
    perror("send");
    close(connfd);
    return -1;
  }
  return res;
}

int recv_msg(int connfd, char *msg, int length) {
  int res = recv(connfd, msg, length, 0);
  if (res < 0) {
    perror("recv");
    close(connfd);
    return -1;
  }
  return res;
}
// 结果判断
void equals(char *pattern, char *result, char *casename) {
  if (strcmp(pattern, result) == 0) {
    /*  printf("==> PASS --> %s\n", casename); */
  } else {
    printf("==> FAILED --> %s, '%s' != '%s'\n", casename, pattern, result);
  }
}

// "SET name king" "SUCCESS"  "SETCase"
//
int test_case(int connfd, char *msg, char *pattern, char *casename) {
  if (msg == NULL || pattern == NULL || casename == NULL) {
    return -1;
  }
  send_msg(connfd, msg, strlen(msg));
  char result[1024] = {0};
  recv_msg(connfd, result, sizeof(result) - 1);
  equals(pattern, result, casename);
}

// array-test
void array_testcase(int connfd) {
  test_case(connfd, "SET Name King", "SUCCESS", "SETCase");
  test_case(connfd, "GET Name", "King", "GETCase");
  test_case(connfd, "MOD Name Darren", "SUCCESS", "MODCase");
  test_case(connfd, "GET Name", "Darren", "GETCase");
  test_case(connfd, "DEL Name", "SUCCESS", "DELCase");
  test_case(connfd, "GET Name", "NOT EXIST", "GETCase");
}

// 循环测试10w次
void array_testcase_10w(int connfd) {
  int count = 100000;
  int i = 0;
  while (i++ < count) {
    array_testcase(connfd);
  }
}

//./testcase -s 192.168.150.130 -p 2048 -m 1
// array: 0x01,rbtree: 0x02, hash: 0x04 skiptable: 0x08
int main(int argc, char *argv[]) {
  int ret = 0;

  char ip[16] = {0};
  int port = 0;
  int mode = 1;

  int opt;
  while ((opt = getopt(argc, argv, "s:p:m:?")) != -1) {
    switch (opt) {
    case 's':
      strcpy(ip, optarg);
      break;

    case 'p':
      port = atoi(optarg);
      break;
    case 'm':
      mode = atoi(optarg);
      break;

    default:
      return -1;
    }
  }

  int connfd = connect_tcpserver(ip, port);
  if (mode & 0x1) {

    struct timeval tv_begin;
    gettimeofday(&tv_begin, NULL);
    array_testcase_10w(connfd);

    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);

    int time_used = TIME_SUB_MS(tv_end, tv_begin);

    printf("time_used: %d, qps: %lld\n", time_used,
           (600000LL * 1000) / time_used);
  }
}
