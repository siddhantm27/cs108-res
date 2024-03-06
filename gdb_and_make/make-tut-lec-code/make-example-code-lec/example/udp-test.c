// Program to send/receive UDP packets (and measure throughput)
// Bhaskaran Raman, bhaskar@cs.berkeley.edu, Nov 14 2000

//#error This is an error

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <netdb.h>
#include <stdlib.h>
#include <assert.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "sockutil.h"
#include "timeutil.h"
#include "sync.h"

#define PORT 2000
#define ALARM_TIME 5
/* The runtime of the program in ms */
#define RUN_TIME 120000

#define SEND_MODE 1
#define RECV_MODE 2
#define BOTH_MODE 3

#define OVERHEAD 68 //bytes

#define MAX_PKT_SIZE 2000
#define SNDBUF_PKTS 120

struct hb_pkt {
  long seqno;
  int fid;	
  char dummy[MAX_PKT_SIZE];
};
typedef struct hb_pkt hb_pkt_t;

/* The time at which heartbeats are started */
struct timeval start;
/* The socket descriptor */
int sock_fd;
/* Address on which to listen */
struct sockaddr_in listen_addr;
/* Port on which to listen */
int listen_port;
/* Address to which to send packets */
struct sockaddr_in to_addr;
/* Node to which to send packets */
char to_host[256];
/* Node to which to send packets */
struct in_addr to_ip;
/* Port to which to send packets */
int to_port;
/* The heart-beat interval (in ms) */
double hbInt;
/* The packet size */
int pkt_size;

long num_recd = 0;
long num_sent = 0;
double prev_stat_time = 0;
long num_lost = 0;
long num_dup = 0;
long next_seqno_expected = 0;
long num_rcv_err = 0;

double prev_pkt_recv_time = 0;
double sum_inter_pkt_time = 0;
double curr_inter_pkt_time = 0;
double prev_inter_pkt_time = 0;
double sum_jitter = 0;
int num_jitter = 0;
int total_pkts_to_send = 100;


void set_sndbuf(int sockfd, int sndbuf_size) {
 int sendbuff;
 socklen_t optlen;

 int res = 0;

 // Get buffer size
 optlen = sizeof(sendbuff);
 res = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbuff, &optlen);

 if(res == -1)
     printf("Error getsockopt one");
 else
     printf("send buffer size = %d\n", sendbuff);

 // Set buffer size
 sendbuff = sndbuf_size;

 printf("sets the send buffer to %d\n", sendbuff);
 res = setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff));

 if(res == -1)
     printf("Error setsockopt");


 // Get buffer size
 optlen = sizeof(sendbuff);
 res = getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbuff, &optlen);

 if(res == -1)
     printf("Error getsockopt two");
 else
     printf("send buffer size = %d\n", sendbuff);

} // End set_sndbuf()

/* Send heartbeat on the socket descriptor */
void sendHB(hb_pkt_t *pkt, int len) {
  fd_set sfds;
  FD_ZERO(&sfds);
  FD_SET(sock_fd, &sfds);
  select(sock_fd+1, NULL, &sfds, NULL, NULL); // block indefinitely
  
  if(FD_ISSET(sock_fd, &sfds)) {  
  	int res = sendto(sock_fd, (void *)(pkt), len, 0,
			   (struct sockaddr *)&to_addr, sizeof(to_addr));

   //     fprintf(stderr, "\n send pkt len = %d", res);

	if(res != len) {
    		perror("sendto");
    	return;
  	}
   }
} // End sendHB()

/* Try to receive heartbeat on the listen address */
int tryReceive(FILE *fp) {
  struct timeval mytime;
  struct timeval t_o;
  t_o.tv_sec = 1;
  t_o.tv_usec = 0; // don't wait around

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(sock_fd, &rfds);
  select(sock_fd+1, &rfds, NULL, NULL, &t_o);

  /* Check if there is some data to be read */
  if(FD_ISSET(sock_fd, &rfds)) {
    int len = sizeof(hb_pkt_t);
    hb_pkt_t pkt;
    unsigned int length = sizeof(listen_addr);
    int res = recvfrom(sock_fd, (void *)(&pkt), len, 0,
		       (struct sockaddr*)&listen_addr, &length);
    
    //fprintf(stderr, "\n recv pkt len = %d", res); 

    if(0 && res != len) {
      //fprintf(stderr, "tryReceive recvfrom failed, res=%d\n", res);
      //perror("recvfrom");
      num_rcv_err++;
      return 0;
    }
    num_recd++;
//    double now_time = now(start);
    gettimeofday(&mytime, NULL);
    double now_time = (double)(mytime.tv_sec*1000.0) + (double)(mytime.tv_usec/1000.0); 

    if(prev_pkt_recv_time != 0) {
	curr_inter_pkt_time = (now_time - prev_pkt_recv_time);
	sum_inter_pkt_time += curr_inter_pkt_time;
    }

    if(prev_inter_pkt_time != 0) {
	double curr_jitter = (curr_inter_pkt_time > prev_inter_pkt_time ? (curr_inter_pkt_time - prev_inter_pkt_time) : (prev_inter_pkt_time - curr_inter_pkt_time));
	if(curr_jitter > 0) {
		num_jitter++;
		sum_jitter += curr_jitter;
	}
    }
	
    prev_pkt_recv_time = now_time;	
    prev_inter_pkt_time = curr_inter_pkt_time;

    long seqno_recd = ntohl(pkt.seqno);
    int id = ntohl(pkt.fid);
//    fprintf(fp,"%f %ld %d %ld %ld \n",now_time,seqno_recd,id,num_lost,num_dup);
    if(seqno_recd > next_seqno_expected) {
      num_lost += seqno_recd - next_seqno_expected;
      next_seqno_expected = seqno_recd+1;
    } else if(seqno_recd < next_seqno_expected) {
      num_dup += next_seqno_expected - seqno_recd;
    } else {
      next_seqno_expected = seqno_recd+1;
    }
    return 1;
  } // End if(FD_ISSET)

  return 0;
} // End tryReceive()

/* Print usage of program */
void usage(char *progname) {
  fprintf(stderr, "%s <-s/-c/-b> <port> <remote-host> <hbInt-in-ms> <bdcst=0/1> <file-name> <num-of-pkts> <id> <size(max 2000)>\n",
	  progname);
} // End usage()

void print_stats() {
  int i;
  double now_time = now(start);
  double time_taken = now_time-prev_stat_time;
  double throughput = (0.001*num_recd*8.0*(pkt_size + OVERHEAD))/time_taken;
//  throughput /= 1024.0; // In Kbps

  for(i = 0; i < 60; i++) { fputc('-', stderr); }
  fprintf(stderr, "\n");
  fprintf(stderr, "Now time\t= %0.6f\n", now_time);
  fprintf(stderr, "Num pkts recd\t= %ld\n", num_recd);
  fprintf(stderr, "Num pkts sent\t= %ld\n", num_sent);
  fprintf(stderr, "Num pkts lost\t= %ld\n", num_lost);
  fprintf(stderr, "Num pkts dup\t= %ld\n", num_dup);
  fprintf(stderr, "Num pkts rcverr\t= %ld\n", num_rcv_err);
  fprintf(stderr, "Time taken\t= %0.4g ms\n", time_taken);
  fprintf(stderr, "Throughput\t= %0.6g Mbps\n", throughput);
  fprintf(stderr, "WARNING: assuming packet size to be %d\n", pkt_size);

  num_sent = num_recd = 0;
  num_lost = num_dup = 0;
  num_rcv_err = 0;
  prev_stat_time = now_time;
} // End print_stats()

void intr_catcher(int x) {
  print_stats();
  fprintf(stderr, "Caught Ctrl-C, exiting...\n");
  exit(0);
} // End intr_catcher()

void alarm_catcher(int x) {
  print_stats();
  alarm(ALARM_TIME);
  //exit(0);
  
} // End alarm_catcher()

inline int size_pattern(int seqno, int master_pkt_size) {
  int rem100 = seqno%100;
  int rem10 = seqno%10;
  int rem2 = seqno%2;
  int size = 0;
  // For master_pkt_size of 1000, pattern is:
  // (800,900) x 5
  // (1000,999,998,997,996,996,997,998,999,1000) x 8
  // (850,950) x 5
  if(rem100 < 10) { size = master_pkt_size-200+100*rem2; }
  else if(rem100 < 90) {
    if(rem10 < 5) { size = master_pkt_size-rem10; }
    else { size = master_pkt_size-9+rem10; }
  } else {
    size = master_pkt_size-150+100*rem2;
  }
  return (size > 0) ? size : master_pkt_size;
} // End size_pattern()

int main(int argc, char *argv[]) {
  FILE *fp;
  
  int isbroadcast = 0, rc, opt;
  
   /* Get command-line arguments */
  if(argc != 10) { usage(argv[0]); exit(11); }
  char mode_str[256];
  strcpy(mode_str, argv[1]);
  int port = atoi(argv[2]);
  strcpy(to_host, argv[3]);
  hbInt = atof(argv[4]);
  char fname[256];
  strcpy(fname,argv[6]);
  int dur = 0; // atoi(argv[7]);
  total_pkts_to_send = atoi(argv[7]);
  int id = atoi(argv[8]);
  pkt_size = atoi(argv[9]);

  if(strcmp(argv[5], "bdcst=0") == 0) { isbroadcast = 0; }
  else if(strcmp(argv[5], "bdcst=1") == 0) { isbroadcast = 1; }
  else { usage(argv[0]); exit(12); }
  
  if(!(fp = fopen(fname,"w")))
    fprintf(stderr,"unable to open file \n");
  
  int mode = 0;
  if(strcmp(mode_str, "-s") == 0) { mode = SEND_MODE; }
  else if(strcmp(mode_str, "-c") == 0) { mode = RECV_MODE; }
  else if(strcmp(mode_str, "-b") == 0) { mode = BOTH_MODE; }
  else { usage(argv[0]); exit(2); }

  to_port = listen_port = port;

  /* socket */
  //sock_fd = socket(PF_INET, SOCK_DGRAM, getprotobyname("udp")->p_proto);
  sock_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(sock_fd < 0) { perror("socket failed"); exit(2); }
  fprintf(stderr, "socket successful\n");
  /* set socket option for broadcast */
  if(isbroadcast == 1){
    rc = setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (void*)&opt, sizeof(opt));
    if(rc<0) {
      perror("setsockopt");
      fprintf(stderr, "Can not set socket option\n");
      exit(4);
    }
    fprintf(stderr, "setsockopt successful\n");
  }
  set_sndbuf(sock_fd, MAX_PKT_SIZE*SNDBUF_PKTS);
  /* bind */
  bzero((char *)&listen_addr, sizeof(listen_addr));
  listen_addr.sin_family =  AF_INET;
  listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  listen_addr.sin_port = htons(listen_port);

  if(bind(sock_fd, (struct sockaddr *)&listen_addr,
	  sizeof(listen_addr)) < 0) {
    fprintf(stderr, "Can't bind to port %d\n", listen_port);
    perror("bind failed"); exit(3);
  }
  fprintf(stderr, "bind successful\n");

  // Activate interrupt catcher
  signal(SIGINT, intr_catcher);
  signal(SIGTERM, intr_catcher);

  signal(SIGALRM, alarm_catcher);
//  alarm(ALARM_TIME);

  /* init to_addr */
  //struct hostent *server = gethostbyname(to_host);
  //bzero((char *)&to_addr, sizeof(to_addr));
  //bcopy((char *)server->h_addr, (char *)&to_addr.sin_addr.s_addr,
  //server->h_length);
  if (inet_aton(to_host, &to_addr.sin_addr) == 0) {
    fprintf(stderr, "Invalid address: %s\n", to_host);
    exit(5);
  }
  to_addr.sin_family = PF_INET;
  to_addr.sin_port = htons(to_port);

  /* Forever, keep sending and receiving heartbeats */
  gettimeofday(&start, NULL);
  double startTime = 1.0*start.tv_sec + 1e-6 * start.tv_usec;
  num_recd = 0;
  prev_stat_time = 0;
  fprintf(stderr, "Start = %f\n", startTime);

  long seqno = 0;
  hb_pkt_t pkt;
  int i;
  int no_recv_pkt;
  double prev_time = 0;
  for(;;) {
    double now_time = now(start);
    // Check if time to exit
//    if(((mode == SEND_MODE) || (mode == BOTH_MODE)) && (now_time >= (dur*1000))) {
    if(((mode == SEND_MODE) || (mode == BOTH_MODE)) && (num_sent >= total_pkts_to_send)) {
      break;
    }
    // Check if time to send next heartbeat
   if(((mode == SEND_MODE) || (mode == BOTH_MODE)) && (now_time >= (seqno*hbInt))) {
      for(i = 0; i < 1; i++) {
	pkt.seqno = htonl(seqno);
	pkt.fid = htonl(id);
	sendHB(&pkt, pkt_size /*size_pattern(seqno, pkt_size)*/);
	seqno++;
	num_sent++;
	prev_time = now_time;
      }
    }
    // Try receiving
    if((mode == RECV_MODE) || (mode == BOTH_MODE)) {
      no_recv_pkt = 1;
      for(i = 0; i < 5; i++) { 
	if(tryReceive(fp)) {
		no_recv_pkt = 0; 
		break; 
	}
      }
	
      if(no_recv_pkt == 1)
	break;
    }
    //usleep(5000); // sleep for 5ms
  } // End for(;;)

  if((mode == SEND_MODE) || (mode == BOTH_MODE)) {
	fprintf(stderr, "\nNum pkts sent = %ld\nSize per pkt including overhead = %d\n", num_sent, pkt_size+OVERHEAD);
  }

  if(((mode == RECV_MODE) || (mode == BOTH_MODE))) {
	fprintf(stderr, "\nTotal pkts recd = %ld\nTotal time in recv = %lf\nSize per pkt including overhead = %d", num_recd, sum_inter_pkt_time, pkt_size + OVERHEAD);
	fprintf(stderr, "\nThroughtput = %lf Mbps, jitter = %lf ms\n\n", 8*0.001*(num_recd-1)*(pkt_size + OVERHEAD)/(sum_inter_pkt_time), sum_jitter/(num_jitter));
  }
//  print_stats();
} // End main()
