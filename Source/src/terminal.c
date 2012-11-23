/*
 ============================================================================
 Name        : terminal.c
 Author      : Greg
 Version     :
 Copyright   : 2012
 Description :
 ============================================================================
 */

#include "utils/comm.h"
#include "utils/media.h"
#include "utils/filesys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include <json/json.h>

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
const char *
parse_input(Socket *sock, char *input) {
	json_object *jobj;
	json_object *action_obj;
	json_object *action_data_obj;
	char action[BUF_SIZE];

	jobj = json_tokener_parse(input);
	action_obj = json_object_object_get(jobj, "action");
	strcpy(action, json_object_get_string(action_obj));
	if (strcmp(action, "discover")) {
		return json_object_to_json_string(jobj);
	} else if (strcmp(action, "getfiles")) {
		action_data_obj = json_object_object_get(jobj, "actiondata");
		return filesys_get(json_object_to_json_string(action_data_obj));
	}

	return NULL ;
}

/*
 * Starts the main UDP server loop for processing commands/messages from clients
 */
void *start_udp_server(void *arg) {
	int received;
	char buf[BUF_SIZE];
	Socket *sock;

	printf("INFO: Creating UDP server\n");
	sock = comm_create_udp_server(BROADCAST_PORT);

	if (sock) {
		printf("INFO: UDP server started\n");
		while (1) {
			memset(&buf, 0, sizeof(buf));
			received = comm_receive(sock, buf, sizeof(buf));
			if (received > 0) {
				printf("DEBUG: echo command - %s\n", buf);
				strcpy(buf, parse_input(sock, buf));
				printf("DEBUG: echo response - %s\n", buf);
				received = comm_send(sock, buf, strlen(buf));
				if (received < 0)
					printf("ERROR: failed sending response\n");
			}
		}

		comm_destroySocket(sock);
	}

	pthread_exit(0);
	return arg;
}

void *start_tcp_server(void *arg) {
	int received;
	char buf[BUF_SIZE];
	Socket *sock;
	Socket *client;

	printf("INFO: Creating TCP socket\n");
	sock = comm_create_tcp_server(TCP_PORT, 1);

	if (sock) {
		printf("INFO: TCP server started\n");

		while (1) {
			client = comm_accept_connection(sock, 5000, sizeof(buf));
			if (client == NULL )
				continue;

			printf("INFO: Client connected - %s", comm_get_cli_address(client));
			while (1) {
				memset(&buf, 0, sizeof(buf));
				received = comm_receive(client, buf, sizeof(buf));
				if (received > 0) {
					printf("DEBUG: echo command - %s\n", buf);
					strcpy(buf, parse_input(sock, buf));
					printf("DEBUG: echo response - %s\n", buf);
					received = comm_send(sock, buf, strlen(buf));
					if (received < 0)
						printf("ERROR: failed sending response\n");
				} else if (received == -1) {
					close(client);
					break;
				}
			}

			printf("INFO: Client disconnected\n");
		}
	}
	pthread_exit(0);
	return arg;
}

/**
 * Starting point of the application
 */
int main(int argc, char *argv[]) {
	pthread_t ucp_server_thread, tcp_server_thread;

	setstdoutput();
	printf("Starting Hawkeye Terminal\n");

	pthread_create(&ucp_server_thread, NULL, start_udp_server, NULL );
	pthread_create(&tcp_server_thread, NULL, start_tcp_server, NULL );

	printf("INFO: All threads started\n");
	pthread_join(ucp_server_thread, NULL );
	pthread_join(tcp_server_thread, NULL );

	printf("INFO: All threads closed\n");

	return EXIT_SUCCESS;
}
