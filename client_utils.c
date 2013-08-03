#include "client_utils.h"
#include <stdio.h>

/* Get the user's name */
char *get_name(void)
{
	int num_bytes;
	
	//Allocate space for the user's name
	char *name = (char *)malloc(sizeof(char) * NAMELEN);
	
	//Prompt the user for their name -- use stderr for unbuffered output
	fprintf(stderr, "Name: ");
	num_bytes = read(STDIN_FILENO, name, NAMELEN);
	
	//Null-terminate the name (and strip off the trailing newline character)
	name[num_bytes - 1] = '\0';
	
	//If the name contains a space, put a null-terminator; don't allow names with spaces
	char *iter;
	for (iter = name; *iter != '\0'; iter++)
		//If the iterator finds a space, replace it with a null character and warn the user before breaking from the loop
		if (*iter == ' ')
		{
			*iter = '\0';
			printf("Warning: Spaces not allowed.  Name shortened to %s.\n", name);
			break;
		}
	
	return name;
}

/* Resolve the listener */
void get_addr_info(const char *HOSTNAME, const char *PORT, struct addrinfo *hints, struct addrinfo **addrs)
{
	if (getaddrinfo(HOSTNAME, PORT, hints, addrs) != 0)
	{
		perror("Resolve listener");
		exit(ERROR_RESOLVING_LISTENER);
	}
}

/* Establish the socket and connect to the host */
void establish_connection(int *new_fd, struct addrinfo *addrs)
{
	//Establish the socket
	if ((*new_fd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol)) == -1)
    {
            perror("Socket");
            exit(ERROR_WITH_SOCKET);
    }
    
    //Connect to the host
    if (connect(*new_fd, addrs->ai_addr, addrs->ai_addrlen) == -1)
    {
            perror("Connect");
            exit(ERROR_WITH_CONNECT);
    }
    
    //Set up TCP Keepalive
    keepalive(new_fd, 0);
}
