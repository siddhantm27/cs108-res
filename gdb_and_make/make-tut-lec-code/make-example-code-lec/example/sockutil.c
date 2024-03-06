#include "sockutil.h"
#include "timeutil.h"

/** Wait for at most timeout_ms, to read 'toreadfd' */
int select_fd_helper(int toreadfd, int timeout_ms) {
  fd_set readfds;
  struct timeval timeout;
  int sock_wait_time_ms = timeout_ms;
  FD_ZERO(&readfds);
  FD_SET(toreadfd, &readfds);
  timeout.tv_sec = sock_wait_time_ms/1000;
  timeout.tv_usec = (sock_wait_time_ms%1000)%1000;

  select(1+toreadfd, &readfds, NULL, NULL, &timeout); /* Wait for message, for timeout */
  return FD_ISSET(toreadfd, &readfds);
} // End select_fd_helper()

//==================================================================//

int make_non_blocking(int s) {
  // MAKE 's' NON-BLOCKING
  int flags = fcntl(s, F_GETFL);
  return fcntl(s, F_SETFL, flags | O_NONBLOCK);
} // End make_non_blocking()

int make_blocking(int s) {
  // MAKE 's' BLOCKING
  int flags = fcntl(s, F_GETFL);
  return fcntl(s, F_SETFL, flags & !O_NONBLOCK);
} // End make_blocking()

void set_sock_buf_size_helper(int sockfd, int size, int optname) {
  int res;
  int check_size, optlen;
  res = setsockopt(sockfd, SOL_SOCKET, optname, &size, sizeof(size));
  if(res < 0) { perror("setsockopt"); exit(70); }
  optlen = sizeof(check_size);
  res = getsockopt(sockfd, SOL_SOCKET, optname, &check_size, &optlen);
  if(res < 0) { perror("getsockopt"); exit(71); }
  if(check_size != size) {
    fprintf(stderr, "Unable to set socket %d size to %d, sock buf size is %d\n", optname, size, check_size);
    exit(72);
  }
} // End set_sock_buf_size_helper()

int set_sock_buf_size(int sockfd, int size) {
  set_sock_buf_size_helper(sockfd, size, SO_RCVBUF);
  set_sock_buf_size_helper(sockfd, size, SO_SNDBUF);
  return 0;
} // End set_sock_buf_size()

int sock_and_listen(int port) {
  int sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock_fd < 0) { perror("socket failed"); return -1; }
  struct sockaddr_in serv_addr;

  /* bind, listen */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);
  if(bind(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("bind"); close(sock_fd); return -1;
  }

  //fprintf(stderr, ",setting SOCK_BUF_SIZE=%d,", SOCK_BUF_SIZE);
  //set_sock_buf_size(sock_fd, SOCK_BUF_SIZE);

  if(listen(sock_fd, SOMAXCONN) < 0) {
    perror("listen"); close(sock_fd); return -1;
  }
  return sock_fd;
} // End sock_and_listen()

int sock_and_connect(struct in_addr server_ip, int port) {
  int sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock_fd < 0) { perror("socket failed"); return -1; }
  struct sockaddr_in serv_addr;

  fprintf(stderr, ",setting SOCK_BUF_SIZE=%d,", SOCK_BUF_SIZE);
  set_sock_buf_size(sock_fd, SOCK_BUF_SIZE);

  /* blocking connect */
  serv_addr.sin_addr.s_addr = server_ip.s_addr;
  serv_addr.sin_family = PF_INET;
  serv_addr.sin_port = htons(port);

  int res = connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if(res == 0) { return sock_fd; } // connection formed
  else { return -1; }

} // End sock_and_connect()

int sock_and_connect_non_blocking(struct in_addr server_ip, int port, double duration) {
  int sock_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if(sock_fd < 0) { perror("socket failed"); return -1; }
  struct sockaddr_in serv_addr;

  /* non-blocking connect */
  serv_addr.sin_addr.s_addr = server_ip.s_addr;
  serv_addr.sin_family = PF_INET;
  serv_addr.sin_port = htons(port);

  struct timeval connect_start;
  gettimeofday(&connect_start, NULL);

  make_non_blocking(sock_fd);

  double remaining_time = duration - now(connect_start);

  while(remaining_time > 0) {
    int res = connect(sock_fd, (struct sockaddr *)&serv_addr,
		      sizeof(serv_addr));
    if(res == 0) { break; } // connection formed

    if(errno == EINPROGRESS) { // need to wait for socket becoming writable
      fd_set wfds;
      FD_ZERO(&wfds);
      FD_SET(sock_fd, &wfds);

      struct timeval tv;
      long remaining_time_long = (long)(remaining_time*1000L);
      tv.tv_sec = remaining_time_long/1000000L;
      tv.tv_usec = remaining_time_long%1000000L;
      int retval = select(sock_fd+1, NULL, &wfds, NULL, &tv);
      if(retval == -1) { perror("select_connect"); exit(150); }

      if(FD_ISSET(sock_fd, &wfds)) {
	int so_err;
	int size = sizeof(int);
	int res1 = getsockopt(sock_fd, SOL_SOCKET, SO_ERROR, (void*)(&so_err), (socklen_t*)&size);
	if(res1 < 0) { perror("getsockopt"); exit(210); }
	if(so_err == 0) { break; } // connection formed
	else { errno = so_err; }
      }
    } // End if(errno == EINPROGRESS)

    if(errno == ECONNREFUSED) { // need to retry after some time
      struct timeval sleep_time;
      sleep_time.tv_sec = 1;
      sleep_time.tv_usec = 0;
      select(0, NULL, NULL, NULL, &sleep_time);      
    }

    remaining_time = duration - now(connect_start);
  } // End while(remaining_time > 0)

  if(remaining_time > 0) {
    make_blocking(sock_fd); // make blocking before returning
    return sock_fd;
  } else {
    close(sock_fd);
    return -1;
  }
} // End sock_and_connect_non_blocking()

void sync_connect(const char *sync_server_ip_str, int sync_port) {
  struct in_addr sync_server_addr;
  if (inet_aton(sync_server_ip_str, &sync_server_addr) == 0) {
    fprintf(stderr, "Invalid IP address: %s\n", sync_server_ip_str);
    exit(51);
  }

  if(sock_and_connect(sync_server_addr, sync_port) < 0) {
    perror("Can't connect with sync server");
    exit(52);
  }
} // End sync_connect()

