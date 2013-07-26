#include "client_setup.h"
#include <stdio.h>

/* Print out the given error message and exit the program with the given error code */
void error(char *message, int err_code)
{
	fprintf(stderr, "%s\n", message);
	exit(err_code);
}

/* Resolve the listener */
void get_addr_info(const char *HOSTNAME, const char *PORT, struct addrinfo *hints, struct addrinfo **addrs)
{
	int err;
	if ((err = getaddrinfo(HOSTNAME, PORT, hints, addrs)) != 0)
	{
		perror("Error getting address info");
		exit(2);
	}
}

/* Establish the socket and connect to the host */
void establish_connection(int *new_fd, struct addrinfo *addrs)
{
	//Establish the socket
	if ((*new_fd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol)) == -1)
    {
            perror("Error establishing socket");
            exit(3);
    }
    
    //Connect to the host
    if (connect(*new_fd, addrs->ai_addr, addrs->ai_addrlen) == -1)
    {
            perror("Error connecting to host");
            exit(4);
    }
    
    //Set up TCP Keepalive
    keepalive(new_fd);
}
