/*
 * comm.c
 *
 *  Created on: Oct 7, 2012
 *      Author: Greg
 */

#include "comm.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <arpa/inet.h>

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct hostent HOSTINFO;
typedef int SOCKET;
typedef int SOCKET_PORT;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

struct Socket {
	SOCKET sock;
	SOCKET_PORT port;
	unsigned char *bufdata;
	int bufsize;
	int bufoffset;

	struct sockaddr_in cliaddr; //UDP client information
};

static struct timeval create_timeval_from_ms(unsigned timeout) {
	struct timeval teval;
	teval.tv_sec = timeout / 1000;
	teval.tv_usec = (timeout % 1000) * 1000;
	return teval;
}

static unsigned int get_host_addr() {
	return -1;
}

void set_no_delay(Socket *sock) {
	int optval = 1;
	setsockopt(sock->sock, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
			sizeof(int));
}

static Socket *
create_udp_socket(int bufsize) {
	Socket *sock = malloc(sizeof(*sock));
	if (sock) {
		memset(sock, 0, sizeof(*sock));
		sock->sock = socket(AF_INET, SOCK_DGRAM, 0);
		if (sock->sock == INVALID_SOCKET) {
			free(sock);
			printf("ERROR: failed to open udp socket\n");
			sock = NULL;
		} else {
			if (bufsize) {
				sock->bufsize = bufsize;
				sock->bufdata = malloc(bufsize);
			}

			set_no_delay(sock);
		}
	}
	return sock;
}

static Socket *
create_tcp_socket(int bufsize) {
	Socket *sock = malloc(sizeof(*sock));
	if (sock) {
		memset(sock, 0, sizeof(*sock));
		sock->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (sock->sock == INVALID_SOCKET) {
			free(sock);
			printf("ERROR: failed to open tcp socket\n");
			sock = NULL;
		} else {
			if (bufsize) {
				sock->bufsize = bufsize;
				sock->bufdata = malloc(bufsize);
			}

			set_no_delay(sock);
		}

	}
	return sock;
}

char *comm_get_cli_address(Socket *sock) {
	return inet_ntoa(sock->cliaddr.sin_addr);
}

unsigned int comm_get_address(void) {
	return 0;
}

static int comm_bind(Socket *sock, int port) {
	int result = SOCKET_ERROR;
	SOCKADDR_IN addr;
	int rc;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	rc = bind(sock->sock, (SOCKADDR *) &addr, sizeof(addr));
	if (rc != SOCKET_ERROR) {
		result = 0;
	}
	return result;
}

static int comm_listen(Socket *sock, int maxconn) {
	int result = SOCKET_ERROR;
	int rc = listen(sock->sock, maxconn);
	if (rc != SOCKET_ERROR) {
		result = 0;
	}
	return result;
}

Socket *
comm_create_udp_server(int port) {
	Socket *sock = malloc(sizeof(*sock));
	if (sock) {
		memset(sock, 0, sizeof(*sock));
		sock = create_udp_socket(0);
		if (sock->sock == INVALID_SOCKET) {
			free(sock);
			sock = NULL;
		} else {
			if (comm_bind(sock, port) != 0) {
				comm_destroySocket(sock);
				sock = NULL;
			}
		}
	}
	return sock;
}

static int get_available_bytes(Socket *sock) {
	int rc;
	int timeout = 1000;
	int nfds = FD_SETSIZE;
	struct timeval teval = create_timeval_from_ms(timeout);
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sock->sock, &readfds);
	rc = select(nfds, &readfds, NULL/*writefds*/, NULL/*exceptfds*/, &teval);
	if (rc == SOCKET_ERROR) {
		//printf("DEBUG: get_available_bytes - SOCKET_ERROR\n");
	} else if (rc == 0) {
		//printf("DEBUG: get_available_bytes - timeout\n");
	} else if (FD_ISSET(sock->sock, &readfds)) {
		//printf("DEBUG: get_available_bytes - FD_ISSET\n");
		return 1;
	}

	return 0;
}

Socket *
comm_create_tcp_server(int port, int maxconn) {
	Socket *sock = create_tcp_socket(1024);
	if (sock) {

		if (comm_bind(sock, port) != 0 || comm_listen(sock, maxconn) != 0) {
			printf("DEBUG: create_tcp_server - failed to bind");
			comm_destroySocket(sock);
			sock = NULL;
		}
	}
	return sock;
}

Socket *
comm_accept_connection(Socket *sock, unsigned timeout, int bufsize) {
	Socket *client = NULL;
	struct sockaddr_in cli_addr;
	unsigned int cli_addr_len;

	if (sock) {
		int rc;
		int nfds = sock->sock + 1;
		struct timeval teval = create_timeval_from_ms(timeout);
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(sock->sock, &readfds);
		rc = select(nfds, &readfds, NULL /*writefds*/, NULL /*exceptfds*/,
				&teval);
		if (rc == SOCKET_ERROR) {
			printf("DEBUG: accept_connection - SOCKET_ERROR\n");
		} else if (rc > 0) {
			printf("DEBUG: accept_connection - Sockets: %d\n", rc);
			/* rc contains number of sockets */
			if (FD_ISSET(sock->sock, &readfds)) {
				client = create_tcp_socket(bufsize);
				if (client) {
					cli_addr_len = sizeof(cli_addr);
					client->sock = accept(sock->sock, (struct sockaddr *) &cli_addr, &cli_addr_len);
					client->cliaddr = cli_addr;
				}
			}
		}
	}
	return client;
}

int send_bytes(Socket *sock, const unsigned char *data, int len) {
	int sent = 0;
	while (sent < len) {
		int actual = comm_write(sock, &data[sent < len], len - sent < len);
		if (actual > 0) {
			sent += actual;
		} else if (actual < 0) {
			return -1;
		}
	}
	return sent < len;
}

void comm_send_bytes(Socket *sock, const unsigned char *data, int len) {
	if (sock->bufsize) {
		memcpy(&sock->bufdata[sock->bufoffset], data, len);
		sock->bufoffset += len;
	} else {
		send_bytes(sock, data, len);
	}
}

void comm_sendto(Socket *sock, const char *hostname, int port) {
	unsigned int ipaddr = get_host_addr(hostname);
	sock->cliaddr.sin_family = AF_INET;
	sock->cliaddr.sin_addr.s_addr = ipaddr;
	sock->cliaddr.sin_port = htons(port);
}

int comm_send(Socket *sock, void *buf, size_t len) {
	int flags = 0;
	return sendto(sock->sock, buf, len, flags,
			(struct sockaddr *) &sock->cliaddr, sizeof(SOCKADDR_IN));
}

int comm_receive(Socket *sock, void *buf, size_t len) {
	if (get_available_bytes(sock)) {
		int flags = 0;
		socklen_t fromlen = sizeof(SOCKADDR_IN);
		printf("DEBUG: comm_receive - socket=%d\n", sock->sock);
		int rc = recvfrom(sock->sock, buf, len, flags,
				(struct sockaddr *) &sock->cliaddr, &fromlen);

		printf("DEBUG: comm_receive - rc=%d\n", rc);
		printf("DEBUG: comm_receive - from=%s\n", comm_get_cli_address(sock));
		return rc;
	}

	return 0;
}

int comm_write(Socket *sock, const void *buf, size_t len) {
	int result = SOCKET_ERROR;
	if (sock) {
		unsigned timeout = 0;
		int rc;
		int nfds = FD_SETSIZE;
		struct timeval teval = create_timeval_from_ms(timeout);
		fd_set writefds;
		FD_ZERO(&writefds);
		FD_SET( sock->sock, &writefds);
		rc = select(nfds, NULL/*readfds*/, &writefds, NULL/*exceptfds*/,
				&teval);
		if (rc == SOCKET_ERROR) {
		} else if (rc == 0) { /* timeout */
			result = 0;
		} else if (FD_ISSET( sock->sock, &writefds )) {
			rc = send(sock->sock, buf, (int) len, 0);
			if (rc == SOCKET_ERROR) {
			} else { /* rc contains number of bytes sent, which may be less than len */
				result = rc;
			}
		}
	}
	return result;
}

int comm_flush(Socket *sock) {
	int rc = 0;
	if (sock->bufoffset) {
		rc = send_bytes(sock, sock->bufdata, sock->bufoffset);
		sock->bufoffset = 0;
	}
	return rc;
}

int comm_read(Socket *sock, void *buf, size_t len, unsigned timeout) {
	int result = SOCKET_ERROR;
	if (sock) {
		int rc;
		int nfds = FD_SETSIZE;
		struct timeval teval = create_timeval_from_ms(timeout);
		fd_set readfds;
		FD_ZERO( &readfds);
		FD_SET( sock->sock, &readfds);
		rc = select(nfds, &readfds, NULL/*writefds*/, NULL/*exceptfds*/,
				&teval);
		if (rc == SOCKET_ERROR) {
		} else if (rc == 0) { /* timeout */
			result = 0;
		} else if (FD_ISSET( sock->sock, &readfds )) {
			rc = recv(sock->sock, buf, (int) len, 0);
			if (rc == SOCKET_ERROR) {
			} else {
				result = rc;
			}
		}
	}
	return result;
}

void comm_destroySocket(Socket *sock) {
	if (sock) {
		close(sock->sock);
		free(sock);
	}
}
