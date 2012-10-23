/*
 ============================================================================
 Name        : terminal.c
 Author      : Greg Beaty
 Version     :
 Copyright   : 2012
 Description :
 ============================================================================
 */

#include "comm.h"
#include "media.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <pthread.h>
//#include <semaphore.h>

#define BROADCAST_PORT 8000
#define STREAM_PORT 15000
#define TCP_PORT 9000
#define BUF_SIZE 1024

/**
 * Sets the output to be displayed in console window
 */
void setstdoutput(void) {
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
}

/**
 * Encapsulates the error logging
 */
void error(char *msg) {
	perror(msg);
	exit(1);
}

int start_stream(Socket *sock) {
	return media_stream_init(comm_get_cli_address(sock), STREAM_PORT);
}

/**
 * Parses the specified string and overwrites it based on the analysis.
 * @param input The string to be parsed.
 */
char *
parse_input(Socket *sock, char *input) {
	if (strcmp(input, "TERM_DISCOVER") == 0) {
		start_stream(sock);
		strcpy(input, "TERM_AVAIL");
	} else if (strcmp(input, "GET_FILE_LIST") == 0) {

	} else {
		strcpy(input, "TERM_UNDEFINED");
	}
	return input;
}

/*
 * Starts the main UDP server loop for processing commands/messages from clients
 */
void *start_udp_server(void *arg) {
	int received;
	char buf[BUF_SIZE];
	char *input;

	printf("DEBUG: creating udp socket\n");
	Socket *sock = comm_create_udp_server(BROADCAST_PORT);

	printf("DEBUG: udp server started\n");
	input = buf;
	while (1) {
		memset(&buf, 0, sizeof(buf));
		received = comm_receive(sock, buf, sizeof(buf));
		if (received > 0) {
			printf("DEBUG: echo command - %s\n", buf);
			input = parse_input(sock, input);
			printf("DEBUG: echo response - %s\n", buf);
			received = comm_send(sock, buf, strlen(buf));
			if (received < 0)
				printf("ERROR: failed sending response\n");
		}
	}

	comm_destroySocket(sock);
}

void *start_tcp_server(void *arg) {
	int received;
	char buf[BUF_SIZE];
	char *input;

	printf("DEBUG: creating tcp socket\n");
	Socket *sock = comm_create_tcp_server(TCP_PORT, 1);

	printf("DEBUG: tcp server started\n");
	while (1) {
		Socket *client = comm_accept_connection(sock, 1000, sizeof(buf));
		if (client == NULL )
			continue;

		printf("DEBUG: client connected - %s", comm_get_cli_address(client));
		while (1) {
			memset(&buf, 0, sizeof(buf));
			received = comm_receive(client, buf, sizeof(buf));
			if (received > 0) {
				printf("DEBUG: echo command - %s\n", buf);
				input = parse_input(sock, input);
				printf("DEBUG: echo response - %s\n", buf);
				received = comm_send(sock, buf, strlen(buf));
				if (received < 0)
					printf("ERROR: failed sending response\n");
			}
		}
	}
}

/**
 * Starting point of the application
 */
int main(int argc, char *argv[]) {
	setstdoutput();

	start_udp_server(NULL);
	//pthread_t ucp_server_thread, tcp_server_thread;
	//pthread_create(&ucp_server_thread, NULL, start_udp_server, NULL );
	//pthread_create(&tcp_server_thread, NULL, start_tcp_server, NULL );

	printf("DEBUG: Waiting for threads to terminate\n");
	//pthread_join(ucp_server_thread, NULL);
	//pthread_join(tcp_server_thread, NULL);

	return EXIT_SUCCESS;
}
