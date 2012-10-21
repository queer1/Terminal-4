/*
 * comm.h
 *
 *  Created on: Oct 7, 2012
 *      Author: Greg
 */

#ifndef COMM_H_
#define COMM_H_

struct Socket;
typedef struct Socket Socket;

#include "media.h"
#include <stddef.h>

#define DEFAULT_PORT 8000 //TODO: Allow port to be configurable from Config Tool

Socket *comm_create_udp_server(int port);
Socket *comm_create_tcp_server(int port, int maxconn);
Socket *comm_accept_connection(Socket *sock, unsigned timeout, int bufsize);
Socket *comm_create_tcp_client(void);
char *comm_get_cli_address(Socket *sock);
unsigned int comm_get_address(void);

void comm_sendTo(Socket *sock, const char *hostname, int port);
int comm_send(Socket *sock, void *buf, size_t len);
int comm_receive(Socket *sock, void *buf, size_t len);
int comm_write(Socket *sock, const void *buf, size_t len);
int comm_flush(Socket *sock);
int comm_read(Socket *sock, void *buf, size_t len, unsigned timeout);

void comm_destroySocket(Socket *sock);

#endif /* COMM_H_ */
