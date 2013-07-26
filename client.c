#include "client_setup.h"

int main(int argc, char *argv[])
{
	//If the user did not specify a hostname and port number, exit
	if (argc != 3)
		error("Usage: ./client <hostname> <port>", 1);
		
	int new_fd, fdmax;
	fd_set master, read;	//Used to monitor all users to create a multiplexed server
	struct addrinfo hints;	//Desired socket properties
	struct addrinfo *addrs;	//Linked list of addresses
	
	//Zero out the fd_sets
	FD_ZERO(&master);
	FD_ZERO(&read);
	
	//Zero out the addrinfo hints
	memset(&hints, 0, sizeof(hints));
	
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
	
	//Get the address info
	get_addr_info(argv[1], argv[2], &hints, &addrs);
	
	//Establish the connection to the host
	establish_connection(&new_fd, addrs);
	
	//Set up the fd_sets
	FD_SET(new_fd, &master);
	FD_SET(STDIN_FILENO, &master);
	
	//Find the initial fdmax
	if (new_fd > STDIN_FILENO)
		fdmax = new_fd;
	else
		fdmax = STDIN_FILENO;
		
	/* TODO: Send initial message to server */
	
	//Main client code.  Loop forever.
	while (IU_SUCKS)
	{
		//Set the read set equal to the master set
		read = master;
		
		//Use multiplexing to select the correct fd
		if (select(fdmax + 1, &read, NULL, NULL, NULL) == -1)
		{
			perror("Error with select");
			exit(5);
		}
		
		//If the STDIN fd is active, read from the user
		if (FD_ISSET(STDIN_FILENO, &read))
		{
			/* TODO: The user sends data here */
		}
	}
}
