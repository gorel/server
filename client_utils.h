#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include <pthread.h>
#include <semaphore.h>
#include "cJSON.h"
#include "generic_utils.h"
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
	int fd;				//the fd to listen on
	sem_t *exit_signal;	//A semaphore to signal the thread should end -- the signal may come from the receiver thread OR the main thread
};

/* Print out the given error message and exit the program with the given error code */
void error(char *message, int err_code);

/* Get the user's name */
char *get_name(void);

/* Resolve the listener */
void get_addr_info(const char *HOSTNAME, const char *PORT, struct addrinfo *hints, struct addrinfo **addrs);

/* Establish the socket and connect to the host */
void establish_connection(int *new_fd, struct addrinfo *addrs);

#endif
