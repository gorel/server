#ifndef CLIENT_SETUP_H
#define CLIENT_SETUP_H

#include "keepalive.h"
#include <pthread.h>
#include "cJSON.h"
#include "return_vals.h"


#define IU_SUCKS 1
#define NAMELEN 20

#define MAXLEN 1024
#define MAXMSG 512

#define QUIT_OPTION 1

#define TRUE 1
#define FALSE 0

typedef char bool;

struct thread_data
{
	int fd;	//the fd to listen on
};

/* Print out the given error message and exit the program with the given error code */
void error(char *message, int err_code);

/* Get the user's name */
char *get_name(void);

/* Resolve the listener */
void get_addr_info(const char *HOSTNAME, const char *PORT, struct addrinfo *hints, struct addrinfo **addrs);

/* Establish the socket and connect to the host */
void establish_connection(int *new_fd, struct addrinfo *addrs);

/* Receive the initial message from the server and output it to console */
bool receive_initial_message(int server_fd);

/* Create an extra thread to output when a new message is received
 * This allows the user to send messages and receive them in real time
 * instead of having an awkward "all-at-once" kind of connection
 */
void *receive(void *thread_data);

/* Send initial data to the server */
void send_initial_message(int server_fd, char *name);

/* Wait for the user to input text then send it to the server */
int send_new_message(int server_fd, char *name);

#endif
