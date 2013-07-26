#include "server_setup.h"

int main(int argc, char *argv[])
{
	//If the user did not specify a port number, exit
	if (argc != 2)
		error("Usage: ./server <port>", 1);

	//The port is the first command line argument
	const char *PORT = argv[1];

	int listen_fd, new_fd, fdmax;
	
	struct addrinfo hints;					//Desired socket properties
	struct addrinfo *addrs;					//Linked list of addresses
	struct sockaddr_storage new_address;	//Address of a user that has not been seen until now
	struct user *users;						//List of users currently connected
	//struct user *userptr;					//Iterator to traverse over the user list
	fd_set master, read;					//Used to monitor all users to create a multiplexed server
		
	//Display the host information
	displayhostinfo(PORT);
	
	//Zero out the fd_sets
	FD_ZERO(&master);
	FD_ZERO(&read);
	
	//Allocate storage for the linked list of users
	users = (struct user *)malloc(sizeof(struct user));
	
	//Zero out the addrinfo hints
	memset(&hints, 0, sizeof(hints));
	
	//Set the properties of hints
	hints.ai_family = AF_INET;			//IPv4 connections
	hints.ai_socktype = SOCK_STREAM;	//TCP
	hints.ai_flags = AI_PASSIVE;		//The host is the local machine
	
	//Resolve the listener
	resolve_listener(PORT, &hints, &addrs);
		
	//Establish the socket
	establish_socket(addrs, &listen_fd);
	
	//Make the server start listening on the port
	if (listen(listen_fd, MAX_WAIT) == -1)
		error("Error with listen.", 5);
		
	//Free addrs
	freeaddrinfo(addrs);
	
	//Update the master fd_set and set fdmax
	FD_SET(listen_fd, &master);
	fdmax = listen_fd;
	
	//Output that the server is up and running
	printf("Server now active.  Waiting for clients to connect.\n");
	
	//Main server code.  Loop forever
	while (EARTH_IS_THIRD_PLANET_FROM_THE_SUN)
	{
		int i;
		
		//Set the read set equal to the master set
		read = master;
		
		//Monitor all currently connected clients
		if (select(fdmax + 1, &read, NULL, NULL, NULL) == -1)
			error("Error with select.", 6);
		
		//Find out which client sent the message
		for (i = 0; i <= fdmax; i++)
		{
			//Active sockfd found
			if (FD_ISSET(i, &read))
			{
				//New connection found
				if (i == listen_fd)
				{
					//Accept the connection and find which fd the user is located at
					new_fd = accept_new_user(listen_fd, &new_address);
					//Add the user to the list of currently connected users
					add_user(new_fd, &new_address, users);
					
					//Update the master fd_set and fdmax
                    FD_SET(new_fd, &master);
                    if (new_fd > fdmax)
                    	fdmax = new_fd;
                    	
                    //Print out that a new connection was accepted
                    printf("New client connected.\n");
				}
				//Otherwise, an existing client sent the message
				else
				{
					/* TODO: Handle incoming messages */
				}
			}//sockfd found
		}//for
	}//while
}//main
