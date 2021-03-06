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
#include "utils/streaming.h"
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
#define BUF_SIZE 8000

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

void *start_stream(void *arg) {
	int error;
	Socket *sock = (Socket *) arg;

	error = streaming_start(0, NULL, comm_get_cli_host_addr(sock), STREAM_PORT);
	if (error) {
		printf("ERROR: Failed to start the streaming service\n");
	}

	return arg;
}

void stop_stream() {
	streaming_stop();
}

void close_relay() {
	printf("Relay closed");
}

/**
 * Parses the specified string and overwrites it based on the analysis.
 * @param input The string to be parsed.
 */
const char *
parse_input(Socket *sock, char *input) {
	json_object *jobj;
	json_object *action;

	jobj = json_tokener_parse(input);
	action = json_object_object_get(jobj, "Action");

	if (action) {
		const char *str = json_object_get_string(action);
		printf("INFO: Looking for action: %s\n", str);

		if (strcmp(str, "Discovery") == 0) {
			return json_object_to_json_string(jobj);
		} else if (strcmp(str, "GetFileList") == 0) {
			return json_object_to_json_string(filesys_get_filelist(jobj));
		} else if (strcmp(str, "GetFile") == 0) {
			return json_object_to_json_string(filesys_get_file(jobj));
		} else if (strcmp(str, "DeleteFile") == 0) {
			return json_object_to_json_string(filesys_delete_file(jobj));
		} else if (strcmp(str, "GetProfiles") == 0) {
			return json_object_to_json_string(filesys_get_profiles());
		} else if (strcmp(str, "SaveProfiles") == 0) {
			return json_object_to_json_string(filesys_save_profiles(jobj));
		} else if (strcmp(str, "CloseRelay") == 0) {
			close_relay();
			//TODO: Does this need a response?
			return "";
		}
	}

	return "";
}

/*
 * Starts the main UDP server loop for processing commands/messages from clients
 */
void *start_udp_server(void *arg) {
	int bytes;
	char buf[BUF_SIZE];
	Socket *sock;

	printf("INFO: Creating UDP server\n");
	sock = comm_create_udp_server(BROADCAST_PORT);

	if (sock) {
		printf("INFO: UDP server started\n");
		while (1) {
			memset(&buf, 0, sizeof(buf));
			bytes = comm_receive(sock, buf, sizeof(buf));
			if (bytes > 0) {
				printf("DEBUG: echo command - %s\n", buf);
				strcpy(buf, parse_input(sock, buf));
				printf("DEBUG: send response - %s\n", buf);
				strcat(buf, "|");
				bytes = comm_send(sock, buf, strlen(buf));
				if (bytes < 0)
					printf("ERROR: failed sending response\n");
			}
		}

		comm_destroySocket(sock);
	}

	return arg;
}

void *start_tcp_server(void *arg) {
	int bytes;
	char buf[BUF_SIZE];
	Socket *sock;
	Socket *client;
	pthread_t stream_thread;

	printf("INFO: Creating TCP socket\n");
	sock = comm_create_tcp_server(TCP_PORT, 1);

	if (sock) {
		printf("INFO: TCP server started\n");

		while (1) {
			client = comm_accept_connection(sock, 5000, sizeof(buf));
			if (client == NULL )
				continue;

			printf("INFO: Client connected - %s\n",
					comm_get_cli_host_info(client));

			pthread_create(&stream_thread, NULL, start_stream, (void *) client);
			while (1) {
				memset(&buf, 0, sizeof(buf));
				bytes = comm_receive(client, buf, sizeof(buf));
				if (bytes > 0) {
					printf("DEBUG: echo command - %s\n", buf);
					strcpy(buf, parse_input(client, buf));
					printf("DEBUG: send response - %s\n", buf);
					strcat(buf, "|");
					bytes = comm_send(client, buf, strlen(buf));
					printf("Sent: %d\n", bytes);
					if (bytes < 0)
						printf("ERROR: failed sending response\n");
				} else if (bytes == -1) {
					break;
				}
			}

			stop_stream();
			pthread_join(stream_thread, NULL );
			printf("INFO: Client disconnected\n");
		}
	}

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
