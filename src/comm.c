/*
 * comm.c
 *
 *  Created on: Sep 22, 2012
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

typedef struct sockaddr SOCKADDR;
typedef int SOCKET;
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

struct conn
{
	SOCKET socket;
	unsigned char *bufdata;
	int bufsize;
	int bufoffset;

	struct sockaddr_in addr;
};

static unsigned int inetaddr(const char *hostname)
{
	unsigned int ipaddr = INADDR_NONE;
	int oct1, oct2, oct3, oct4;
	if (sscanf(hostname, "%d.%d.%d.%d", &oct1, &oct2, &oct3, &oct4) == 4)
	{
		ipaddr = (oct1 << 24) | (oct2 << 16) | (oct3 << 8) | (oct4 << 0);
		ipaddr = htonl(ipaddr);
	}
	return ipaddr;
}

static unsigned int hostnametoipaddr(const char *pHostname)
{
	unsigned int addr = INADDR_NONE;
	if (pHostname)
	{
		addr = inetaddr(pHostname);
	}
	if (addr == INADDR_NONE)
	{
		struct hostent *hostinfo = gethostbyname(pHostname);
		if (hostinfo)
		{
			memcpy(&addr, hostinfo->h_addr_list[0], hostinfo->h_length);
		}
	}
	return addr;
}

static struct timeval createtimefromms(unsigned timeoutMS)
{
	struct timeval timeout;
	timeout.tv_sec = timeoutMS / 1000;
	timeout.tv_usec = (timeoutMS % 1000) * 1000;
	return timeout;
}

static int bytesavailable(conn *pConn)
{
	int received;
	int timeoutMS = 1000;
	int nfds = FD_SETSIZE;
	struct timeval timeout = createtimefromms(timeoutMS);
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(pConn->socket, &readfds);
	received = select(nfds, &readfds, NULL, NULL, &timeout);
	if (received == SOCKET_ERROR)
	{

	}
	else if (received == 0)
	{
		//timeout
	}
	else if (FD_ISSET(pConn->socket, &readfds))
	{
		return 1;
	}
	return 0;
}

int comm_write(conn *pConn, const void *buf, size_t len)
{
	int result = -1;
	if (pConn)
	{
		unsigned timeoutMS = 0;
		int received;
		int nfds = FD_SETSIZE;
		struct timeval timeout = createtimefromms(timeoutMS);
		fd_set writefds;
		FD_ZERO(&writefds);
		FD_SET(pConn->socket, &writefds);
		received = select(nfds, NULL, &writefds, NULL, &timeout);
		if (received == SOCKET_ERROR)
		{

		}
		else if (received == 0)
		{
			//timeout
			result = 0;
		}
		else if (FD_ISSET(pConn->socket, &writefds))
		{
			received = send(pConn->socket, buf, (int) len, 0);
			if (received == SOCKET_ERROR)
			{

			}
			else
			{
				result = received;
			}
		}
	}
	return result;
}

int sendbytes(conn *pConn, const unsigned char *data, int len)
{
	int sent = 0;
	while (sent < len)
	{
		int actual = comm_write(pConn, &data[sent], len - sent);
		if (actual > 0)
		{
			sent += actual;
		}
		else if (actual < 0)
		{
			return -1;
		}
	}
	return sent;
}

static void destroysocket(conn *pConn)
{
	close(pConn->socket);
	free(pConn);
}

static int comm_bind(conn *pConn, int port)
{
	int result = -1;
	struct sockaddr_in addr;
	int received;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;
	received = bind(pConn->socket, (struct sockaddr*) &addr, sizeof(addr));
	if (received != SOCKET_ERROR)
	{
		result = 1;
	}
	return result;
}

conn*
comm_createudpserver(int port)
{
	conn* pConn = malloc(sizeof(*pConn));
	if (pConn)
	{
		memset(pConn, 0, sizeof(*pConn));
		pConn->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (pConn->socket == INVALID_SOCKET)
		{
			free(pConn);
			pConn = NULL;
		}
		else
		{
			comm_bind(pConn, port);
		}
	}
	return pConn;
}

static int comm_listen(conn *pConn, int maxConnections)
{
	int result = -1;
	int received = listen(pConn->socket, maxConnections);
	if (received != SOCKET_ERROR)
	{
		result = 0;
	}
	return result;
}

void comm_sendto(conn *pConn, const char *hostname, int extport)
{
	unsigned int expaddr = hostnametoipaddr(hostname);
	pConn->addr.sin_family = AF_INET;
	pConn->addr.sin_addr.s_addr = expaddr;
	pConn->addr.sin_port = htons(extport);
}

int comm_send(conn *pConn, void *buf, size_t len)
{
	int flags = 0;
	return sendto(pConn->socket, buf, len, flags,
			(struct sockaddr*) &pConn->addr, sizeof(struct sockaddr_in));
}

int comm_receive(conn *pConn, void *buf, size_t len)
{
	if (bytesavailable(pConn))
	{
		socklen_t remoteLen = sizeof(struct sockaddr_in);
		int flags = 0;
		int received = recvfrom(pConn->socket, buf, len, flags,
				(struct sockaddr*) &pConn->addr, &remoteLen);
		return received;
	}

	return 0;
}

void comm_destroysocket(conn *pConn)
{
	if (pConn)
	{
		destroysocket(pConn);
	}
}

int comm_read(conn *pConn, void *buf, size_t len, unsigned timeoutMS)
{
	int result = -1;
	if (pConn)
	{
		int received;
		int nfds = FD_SETSIZE;
		struct timeval timeout = createtimefromms(timeoutMS);
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(pConn->socket, &readfds);
		received = select(nfds, &readfds, NULL, NULL, &timeout);
		if (received == SOCKET_ERROR)
		{

		}
		else if (received == 0)
		{
			//timeout
			result = 0;
		}
		else if (FD_ISSET(pConn->socket, &readfds))
		{
			received = recv(pConn->socket, buf, (int) len, 0);
			if (received == SOCKET_ERROR)
			{

			}
			else
			{
				result = received;
			}
		}
	}
	return result;
}

void comm_readbytes(conn *pConn, unsigned char *buf, int reqbytes)
{
	int received = 0;
	while (received < reqbytes)
	{
		int temp = comm_read(pConn, &buf[received], reqbytes - received, 100);
		if (temp > 0)
		{
			received += temp;
		}
	}
}

void comm_sendbytes(conn *pConn, const unsigned char *data, int len)
{
	if (pConn->bufsize)
	{
		memcpy(&pConn->bufdata[pConn->bufoffset], data, len);
		pConn->bufoffset += len;
	}
	else
	{
		sendbytes(pConn, data, len);
	}
}

int comm_flush(conn *pConn)
{
	int received = 0;
	if (pConn->bufoffset)
	{
		received = sendbytes(pConn, pConn->bufdata, pConn->bufoffset);
		pConn->bufoffset = 0;
	}
	return received;
}
