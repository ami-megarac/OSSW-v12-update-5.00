#ifndef _NETWORK_H_
#define _NETWORK_H_
#include "first.h"

#include "server.h"

void network_accept_tcp_nagle_disable(int fd);

int network_write_chunkqueue(server *srv, connection *con, chunkqueue *c, off_t max_bytes);

int network_init(server *srv);
int network_close(server *srv);

int network_register_fdevents(server *srv);
int Network_SSL_ReInit(server *srv);
#endif
