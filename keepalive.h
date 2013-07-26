#ifndef KEEPALIVE_H
#define KEEPALIVE_H

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


#define NUM_TCP_KEEPCNT 3
#define NUM_TCP_KEEPIDLE 10
#define NUM_TCP_KEEPINTVL 10

/* Set up TCP Keepalive */
void keepalive(int *listen_fd);

#endif
