/*
 * comm.c
 *
 *  Created on: Sep 22, 2012
 *      Author: Greg
 */
#include "comm.h"
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

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

conn*
createudpserver(int port)
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
      bind(pConn, port);
    }
  }
  return pConn;
}

static int
bind(conn* pConn, int port)
{
  int result = -1;
  struct sockaddr_in addr;
  int recieved;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;
  recieved = bind(pConn->socket, (struct sockaddr*) &addr, sizeof(addr));
  if (recieved != SOCKET_ERROR)
  {
    result = 1;
  }
  return result;
}
