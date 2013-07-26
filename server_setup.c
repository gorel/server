#include "server_setup.h"
#include <stdio.h>

/* Print out the given error message and exit the program with the given error code */
void error(char *message, int err_code)
{
	fprintf(stderr, "%s\n", message);
	exit(err_code);
}

/* Output the server's hostname to standard output */
void displayhostinfo(const char *PORT)
{
	char hostname[1024];
	hostname[1023] = '\0';
	gethostname(hostname, 1023);
	printf("Hostname: %s\n", hostname);
	printf("On Port:  %s\n", PORT);
}

/* Resolve the listener */
void resolve_listener(const char *PORT, struct addrinfo *hints, struct addrinfo **addrs)
{
	int err;
	if ((err = getaddrinfo(NULL, PORT, hints, addrs)) != 0)
		error("Error resolving listener", 2);
}

/* Establish the server's socket */
void establish_socket(struct addrinfo *addrs, int *listen_fd)
{
	//This is needed because setting socket options requires a pointer to the value it is being set to
	int yes = 1;
	
	//Used to iterate over the list of addresses
	struct addrinfo *iter;
	
	for (iter = addrs; iter != NULL; iter = iter->ai_next)
	{
		if ((*listen_fd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol)) == -1)
		{
			error("Error establishing socket", 3);
		}
		else
		{
			//Allow a port to be reused immediately upon close restart
			if (setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
				perror("Reusable socket");
			//Bind the socket to the specified port
			if (bind(*listen_fd, iter->ai_addr, iter->ai_addrlen) < 0)
				perror("Bind");
			//Socket binding setup successful.  Exit the loop.
			else break;
		}
	}
	
	//Set up TCP Keepalive
	keepalive(listen_fd);
	
	//If iter is NULL, there was a failure to bind to any address
	if (iter == NULL)
		error("Failed to bind.", 4);
			
}

/* Accept a new user and return the file descriptor pointing to their connection */
int accept_new_user(int listen_fd, struct sockaddr_storage *new_address)
{
	int new_fd;
	socklen_t addrLen = sizeof new_address;
	char ipv4[INET_ADDRSTRLEN];	
	
	//Accept the new connection
	if ((new_fd = accept(listen_fd, (struct sockaddr *)new_address, &addrLen)) == -1)
	{
		    perror("Accept");
		    return 2;
	}

	//Find the string representation of the given socket address
	if (inet_ntop(AF_INET, &(((struct sockaddr_in *)new_address)->sin_addr), ipv4, INET_ADDRSTRLEN) == NULL)
	if (inet_ntop(AF_INET, &(((struct sockaddr_in *)&new_address)->sin_addr), ipv4, INET_ADDRSTRLEN) == NULL)
	{
		    perror("Network to printable");
		    return 2;
	}
	
	return new_fd;
}

/* Add a user with a given fd and sockaddr to the list of users */
void add_user(int fd, struct sockaddr_storage *address, struct user *users)
{
	struct user *temp;
	struct user *userptr;
    
    //Allocate memory for the new user
    temp = (struct user *)malloc(sizeof(struct user));
    
    //Set the default information for this new user
    temp->fd = fd;
    temp->addr = *((struct sockaddr *)address);
    temp->next = NULL;
    
    //Go to the end of the linked list
    for (userptr = users; userptr->next != NULL; userptr = userptr->next);
    
    //Add the new user to the list of users
    userptr->next = temp;
}
