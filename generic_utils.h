#ifndef GENERIC_UTILS_H
#define GENERIC_UTILS_H

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <assert.h>

#include "cJSON.h"


#define NUM_TCP_KEEPCNT 3
#define NUM_TCP_KEEPIDLE 10
#define NUM_TCP_KEEPINTVL 10

typedef char bool;

/* Set up TCP Keepalive */
void keepalive(int *listen_fd, int output_on);

/* Allocate a JSON string with the given data */
char *allocate_json_string(char *from, char *msg, bool valid, bool private, bool kicked);

#endif
