#ifndef CLIENT_SETUP_H
#define CLIENT_SETUP_H

#include "keepalive.h"

#define IU_SUCKS 1

/* Print out the given error message and exit the program with the given error code */
void error(char *message, int err_code);

/* Resolve the listener */
void get_addr_info(const char *HOSTNAME, const char *PORT, struct addrinfo *hints, struct addrinfo **addrs);

/* Establish the socket and connect to the host */
void establish_connection(int *new_fd, struct addrinfo *addrs);

#endif
