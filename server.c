#include "server_messaging.h"	

int main(int argc, char *argv[])
{
	//If the user did not specify a port number, exit
	if (argc != 2)
	{
		fprintf(stderr, "Usage: ./server <port>\n");
		return INCORRECT_ARG_COUNT;
	}

	//The port is the first command line argument
	const char *PORT = argv[1];

	int listen_fd, new_fd, fdmax, num_bytes;
	
	char msg[MAXLEN];
	
	struct addrinfo hints;					//Desired socket properties
	struct addrinfo *addrs;					//Linked list of addresses
	struct sockaddr_storage new_address;	//Address of a user that has not been seen until now
	struct user *users = NULL;				//List of users currently connected
	fd_set master, read;					//Used to monitor all users to create a multiplexed server
		
	//Display the host information
	displayhostinfo(PORT);
	
	//Zero out the fd_sets
	FD_ZERO(&master);
	FD_ZERO(&read);
	
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
	listen_on_fd(&listen_fd);
		
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
		{
			perror("Select");
			exit(ERROR_WITH_SELECT);
		}
		
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
					add_user(new_fd, &new_address, &users);
					
					//Update the master fd_set and fdmax
                    FD_SET(new_fd, &master);
                    if (new_fd > fdmax)
                    	fdmax = new_fd;
				}
				//Otherwise, an existing client sent the message
				else
				{
					//Receive the next message
					 if ((num_bytes = recv(i, msg, MAXLEN - 1, 0)) == -1)
					 {
                            perror("Receive");
                            exit(ERROR_WITH_RECV);
                     }
                            
                    //Make sure the message is null-terminated
                    msg[num_bytes] = '\0';
                    
                    //Parse the JSON message
                    cJSON *recvJSON = cJSON_Parse(msg);
                    
                    //Find out which user sent the message
                    struct user *sender = get_user_by_fd(users, i);
                    
                    //Handle the message sent
                    handle_message(&users, sender, recvJSON, &master);
                    
                    //Delete the cJSON Object
                    cJSON_Delete(recvJSON);
				}
			}//sockfd found
		}//for
	}//while
	return 0;
}//main
