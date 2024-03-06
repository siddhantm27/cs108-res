#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <sys/time.h>

//#define SOCK_BUF_SIZE (256*1024)
#define SOCK_BUF_SIZE (262142)
//#define SOCK_BUF_SIZE (225280)

double now(struct timeval start);
int select_fd_helper(int toreadfd, int timeout_ms);
int make_non_blocking(int s);
int make_blocking(int s);
int set_sock_buf_size(int sockfd, int size);
int sock_and_listen(int port);
int sock_and_connect(struct in_addr server_ip, int port);
int sock_and_connect_non_blocking(struct in_addr server_ip, int port, double duration);

#endif
