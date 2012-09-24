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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 8000 //TODO: Allow port to be configurable from Config Tool
#define BUF_SIZE 1024
#define DEBUG 1 //TODO: Allow debug mode to be configurable from Config Tool
/**
 * Encapsulates the debug logging
 */
void debug(char *msg)
{
	if (DEBUG)
		return;

	printf("DEBUG: ");
	printf(msg);
	printf("\n");
}

/**
 * Encapsulates the error logging
 */
void error(char *msg)
{
	perror("ERROR: ");
	perror(msg);
	perror("\n");
	exit(1);
}

/*
 * Starts the main UDP server loop for processing commands/messages from clients
 */
void startserver(void)
{
	int sockfd; //socket
	const int port = DEFAULT_PORT; //socket's port
	struct sockaddr_in servaddr, cliaddr; //server and client address
	struct hostent *hostp; //client info
	char buf[BUF_SIZE]; //message buffer
	char *hostaddrp; //human-friendly address
	int optval; //setsockopt flag
	int n; //message byte size

	//creates the server socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd)
	{
		error("socket failed to open");
	}

	//allows the server to run immediately after killing it
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
			sizeof(int));

	//define the server's address
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY );
	servaddr.sin_port = htons(port);

	//binds the socket
	if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
	{
		error("socket failed to bind");
	}

	debug("server started");

	while (1)
	{
		//get UDP datagram from client
		int len = sizeof(cliaddr);
		memset(&buf, 0, sizeof(buf));
		n = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &cliaddr,
				&len);
		if (n < 0)
		{
			error("invalid command");
		}

		//get sender of datagram for logging purposes
		hostp = gethostbyaddr((const char *) &cliaddr.sin_addr.s_addr,
				sizeof(cliaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL )
		{
			error("failed to get client's host info");
		}

		hostaddrp = inet_ntoa(cliaddr.sin_addr);
		if (hostaddrp == NULL )
		{
			error("failed to convert the client's address");
		}

		printf("INFO: received datagram from %s (%s)", hostp->h_name,
				hostaddrp);
		printf("INFO: received %d/%d bytes: %s\n", strlen(buf), n, buf);

		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &cliaddr,
				sizeof(cliaddr));
		if (n < 0)
		{
			error("failed sending response");
		}
	}

	close(sockfd);
}

/**
 * Starting point of the application
 */
int main(void)
{
	startserver();
	return EXIT_SUCCESS;
}
