/*
 ============================================================================
 Name        : terminal.c
 Author      : Greg Beaty
 Version     :
 Copyright   : 2012
 Description :
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "comm.h"

conn *pConn;

void startserver(void)
{
	const int port = 8000; //TODO: change to something more suitable

	if (!pConn)
	{
		pConn = comm_createudpserver(port);
		if (pConn)
		{
			printf("Server started\n");
		}
		else
		{
			printf("Server failed to start");
		}
	}
}

void closeserver(void)
{
	if (pConn)
	{
		comm_destroysocket(pConn);
		printf("Server closed");
	}
}

int main(void)
{
	startserver();
	closeserver();

	return EXIT_SUCCESS;
}
