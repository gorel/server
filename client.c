#include "keepalive.h"
#include "client_utils.h"
#include "client_messaging.h"

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
	
	sem_t exit_signal;			//Semaphore to show when the thread should exit
	
	//Zero out the fd_sets
	FD_ZERO(&master);
	FD_ZERO(&read);
	
	//Zero out the addrinfo hints
	memset(&hints, 0, sizeof(hints));
	
	//Set up the hints for the connection
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
		
		//If the name isn't valid, free it now or suffer a memory leak
		if (!valid)
			free(name);
	}
	
	//Tell the user they have connected to the chat server
	printf("You are now connected to the chat server.  Say hello!\n");
	
	//The thread that will receive messages
	pthread_t receiver_thread;
	
	//Create a semaphore so the main thread and receiver thread can communicate that the other should quit (the program has ended)
	if (sem_init(&exit_signal, 0, 1) != 0)
	{
		perror("Sem init");
		exit(ERROR_SEM_INIT);
	}
	
	//Allocate space for the information to pass into the pthread
	struct thread_data *args = (struct thread_data *)malloc(sizeof(struct thread_data));
	
	//Fill in the args data
	args->fd = server_fd;
	args->exit_signal = &exit_signal;
	
	//Start the receiver thread
	pthread_create(&receiver_thread, NULL, receive, (void *)args);
	
	//Main client code.  Loop forever.
	while (IU_SUCKS)
	{
		int sem_val;
		
		//Get the current semaphore value and store it in sem_val
		sem_getvalue(&exit_signal, &sem_val);
	
		//If the semaphore has been decremented to 0, exit the while loop (receiver thread sent exit signal)
		if (sem_val == 0)
			break;
	
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
			{
				//Decrement the semaphore to signal the receiver thread to end then break out of the while loop
				sem_wait(&exit_signal);
				break;
			}
		}
	}
	
	//Wait for the receiving thread to terminate
	pthread_join(receiver_thread, NULL);
	
	//Free all allocated memory and destroy the semaphore
	free(name);
	free(addrs);
	free(args);
	sem_destroy(&exit_signal);
	
	//Print that the program is exiting and return 0
	printf("Exiting...\n");
	return 0;
}
