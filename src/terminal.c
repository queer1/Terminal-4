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

#define DEFAULT_PORT 8000 //TODO: Allow port to be configurable from Config Tool
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
	return media_stream_init(0, NULL, comm_get_cli_address(sock), DEFAULT_PORT);
}

/**
 * Parses the specified string and overwrites it based on the analysis.
 * @param input The string to be parsed.
 */
char *
parse_input(Socket *sock, char *input) {
	if (strcmp(input, "TERM_DISCOVER") == 0)
		strcpy(input, "TERM_AVAIL");
	else if (strcmp(input, "START_STREAM") == 0) {
		if (start_stream(sock) == 0)
			strcpy(input, "STREAM_STARTED");
		else
			strcpy(input, "STREAM_FAILED");
	} else
		strcpy(input, "TERM_UNDEFINED");
	return input;
}

/*
 * Starts the main UDP server loop for processing commands/messages from clients
 */
void start_udp_server(void) {
	int received;
	char buf[BUF_SIZE];
	char *input;

	printf("DEBUG: creating udp socket\n");
	Socket *sock = comm_create_udp_server(DEFAULT_PORT);

	printf("DEBUG: server started\n");
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

void start_tcp_server(void) {
	return;
}

/**
 * Starting point of the application
 */
int main(int argc, char *argv[]) {
	setstdoutput();
	start_udp_server();
	start_tcp_server();
	return EXIT_SUCCESS;
}
