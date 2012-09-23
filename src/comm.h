/*
 * comm.h
 *
 *  Created on: Sep 22, 2012
 *      Author: Greg
 */

#ifndef COMM_H_
#define COMM_H_

struct conn;
typedef struct conn conn;
#include <stddef.h>

conn* comm_createudpserver(int port);
void comm_sendto(conn *pConn, const char *hostname, int port);
int comm_send(conn *pConn, void *buf, size_t len);
int comm_receive(conn *pConn, void *buf, size_t len);
void comm_destroysocket(conn *pConn);

int comm_read(conn *pConn, void *buf, size_t len, unsigned timeoutMS);
int comm_write(conn *pConn, const void *buf, size_t len);
void comm_readbytes(conn *pConn, unsigned char *buf, int reqbytes);
void comm_sendbytes(conn *pConn, const unsigned char *data, int len);
int comm_flush(conn *pConn);

#endif /* COMM_H_ */
