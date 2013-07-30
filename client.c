#include "client_setup.h"

int main(int argc, char *argv[])
{
	//If the user did not specify a hostname and port number, exit
	if (argc != 3)
	{
		fprintf(stderr, "Usage: ./client <hostname> <port>\n");
		return INCORRECT_ARG_COUNT;
	}
	
	bool valid = FALSE;		//Whether or not the chosen username was valid
	char *name = NULL;		//The user's desired username
	int server_fd, fdmax;	//The server's fd and the maximum of the fd_set
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
	
	//Get the address info -- argv[1] = hostname, argv[2] = port number
	get_addr_info(argv[1], argv[2], &hints, &addrs);
	
	//Establish the connection to the host
	establish_connection(&server_fd, addrs);
	
	//Set up the fd_sets
	FD_SET(server_fd, &master);
	FD_SET(STDIN_FILENO, &master);
	
	//Find the fdmax
	if (server_fd > STDIN_FILENO)
		fdmax = server_fd;
	else
		fdmax = STDIN_FILENO;
	
	//While the username isn't valid, keep getting a new name
	while(!valid)
	{
		//Get the user's name
		name = get_name();

		//Send the initial message to the server
		send_initial_message(server_fd, name);

		//Get the server's response here and find out if the username is valid
		valid = receive_initial_message(server_fd);
	}
	
	//Tell the user they have connected to the chat server
	printf("You are now connected to the chat server.  Say hello!\n");
	
	//The thread that will receive messages
	pthread_t receiverThread;
	
	//Allocate space for the information to pass into the pthread
	struct thread_data *args = (struct thread_data *)malloc(sizeof(struct thread_data));
	args->fd = server_fd;
	
	//Start the receiver thread
	pthread_create(&receiverThread, NULL, receive, (void *)args);
	
	//Main client code.  Loop forever.
	while (IU_SUCKS)
	{
		//Set the read set equal to the master set
		read = master;
		
		//Use multiplexing to select the correct fd
		if (select(fdmax + 1, &read, NULL, NULL, NULL) == -1)
		{
			perror("Select");
			exit(ERROR_WITH_SELECT);
		}
		
		//If the STDIN fd is active, read from the user
		if (FD_ISSET(STDIN_FILENO, &read))
		{
			// Send a message to the server -- if the user chose to quit, break out of the loop
			if (send_new_message(server_fd, name) == QUIT_OPTION)
				break;
		}
	}
	
	//Print that the program is exiting and return 0
	printf("Exiting...\n");
	return 0;
}
