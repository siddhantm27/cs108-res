#ifndef __SYNC_H__
#define __SYNC_H__

#define TCP_SYNC_PORT 1900
#define SYNC_IP ("192.168.1.245")

void sync_connect(const char *sync_server_ip_str, int sync_port);

#endif
