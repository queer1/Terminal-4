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

/**
 * Sets the output to be displayed in console window
 */
void setstdoutput(void)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

/**
 * Encapsulates the error logging
 */
void error(char *msg)
{
	perror(msg);
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
	printf("DEBUG: creating server socket\n");
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR: socket failed to open");

	//allows the server to run immediately after killing it
	printf("DEBUG: setting socket's options\n");
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
			sizeof(int));

	//define the server's address
	printf("DEBUG: defining server's address\n");
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY );
	servaddr.sin_port = htons(port);

	//binds the socket
	printf("DEBUG: binding socket\n");
	if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
		error("ERROR: socket failed to bind");

	printf("DEBUG: server started\n");
	while (1)
	{
		//get UDP datagram from client
		int len = sizeof(cliaddr);
		memset(&buf, 0, sizeof(buf));
		n = recvfrom(sockfd, buf, BUF_SIZE, 0, (struct sockaddr *) &cliaddr,
				&len);
		if (n < 0)
			error("ERROR: invalid command");

		//get sender of datagram for logging purposes
		hostp = gethostbyaddr((const char *) &cliaddr.sin_addr.s_addr,
				sizeof(cliaddr.sin_addr.s_addr), AF_INET);
		if (hostp == NULL )
			error("ERROR: failed to get client's host info");

		hostaddrp = inet_ntoa(cliaddr.sin_addr);
		if (hostaddrp == NULL )
			error("ERROR: failed to convert the client's address");

		printf("INFO: received datagram from %s (%s)\n", hostp->h_name,
				hostaddrp);
		printf("INFO: received %d/%d bytes: %s\n", strlen(buf), n, buf);

		//TODO: Encapsulate
		if (strcmpi(buf, "TERM_DISCOVER") == 0)
			strcpy(buf, "TERM_AVAIL");
		else
			strcpy(buf, "TERM_UNDEFINED");

		n = sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *) &cliaddr,
				sizeof(cliaddr));
		if (n < 0)
			error("ERROR: failed sending response");
	}

	close(sockfd);
}

/**
 * Starting point of the application
 */
int main(void)
{
	//set standard output
	setstdoutput();

	//start the udp server
	startserver();
	return EXIT_SUCCESS;
}
