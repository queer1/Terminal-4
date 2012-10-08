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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PORT 8000 //TODO: Allow port to be configurable from Config Tool
#define BUF_SIZE 1024

/**
 * Sets the output to be displayed in console window
 */
void
setstdoutput(void)
{
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);
}

/**
 * Encapsulates the error logging
 */
void
error(char *msg)
{
  perror(msg);
  exit(1);
}

/**
 * Parses the specified string and overwrites it based on the analysis.
 * @param input The string to be parsed.
 */
void
parse_input(char *input)
{
  if (strcmp(input, "TERM_DISCOVER") == 0)
    strcpy(input, "TERM_AVAIL");
  else
    strcpy(input, "TERM_UNDEFINED");
}

/*
 * Starts the main UDP server loop for processing commands/messages from clients
 */
void
start_udp_server(void)
{
  int received;
  char *buf;

  printf("DEBUG: creating udp socket\n");
  Socket *sock = comm_create_udp_server(DEFAULT_PORT);

  printf("DEBUG: server started\n");
  while (1)
    {
      printf("DEBUG: waiting for command\n");
      received = comm_receive(sock, buf, BUF_SIZE);
      if (received < 0)
        printf("ERROR: invalid command\n");

      parse_input(buf);
      received = comm_send(sock, buf, BUF_SIZE);
      if (received < 0)
        printf("ERROR: failed sending response\n");
    }

  comm_destroySocket(sock);
}

void
start_tcp_server(void)
{
  return;
}

/**
 * Starting point of the application
 */
int
main(void)
{
  setstdoutput();
  start_udp_server();
  start_tcp_server();
  return EXIT_SUCCESS;
}
