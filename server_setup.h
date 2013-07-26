#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <sys/select.h>
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
#include "keepalive.h"

#define MAX_WAIT 10
#define MAXMSG 512

#define EARTH_IS_THIRD_PLANET_FROM_THE_SUN 1

struct user
{
	int fd;
	char *name;
	struct sockaddr addr;
	
	struct user *next;
};

/* Print out the given error message and exit the program with the given error code */
void error(char *message, int err_code);

/* Output the server's hostname to standard output */
void displayhostinfo(const char *PORT);

/* Resolve the listener */
void resolve_listener(const char *PORT, struct addrinfo *hints, struct addrinfo **addrs);

/* Establish the server's socket */
void establish_socket(struct addrinfo *addrs, int *listen_fd);

/* Accept a new user and return the file descriptor pointing to their connection */
int accept_new_user(int listen_fd, struct sockaddr_storage *new_address);

/* Add a user with a given fd and sockaddr to the list of users */
void add_user(int fd, struct sockaddr_storage *address, struct user *users);

#endif
