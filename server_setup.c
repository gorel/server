#include "server_setup.h"
#include <stdio.h>

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
	if (getaddrinfo(NULL, PORT, hints, addrs) != 0)
	{
		perror("Resolve listener");
		exit(ERROR_RESOLVING_LISTENER);
	}
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
			perror("Socket");
			exit(ERROR_WITH_SOCKET);
		}
		else
		{
			//Allow a port to be reused immediately upon close restart
			if (setsockopt(*listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1)
			{
				perror("Reusable socket");
				exit(ERROR_SETSOCKOPT);
			}
			
			//Bind the socket to the specified port
			if (bind(*listen_fd, iter->ai_addr, iter->ai_addrlen) < 0)
			{
				perror("Bind");
				exit(ERROR_WITH_BIND);
			}
			
			//Socket binding setup successful.  Exit the loop.
			else break;
		}
	}
	
	//Set up TCP Keepalive
	keepalive(listen_fd, 0);
	
	//If iter is NULL, there was a failure to bind to any address
	if (iter == NULL)
	{
		perror("Bind");
		exit(ERROR_WITH_BIND);
	}
			
}

/* Start the server listening on the given fd */
void listen_on_fd(int *listen_fd)
{
	if (listen(*listen_fd, MAX_WAIT) == -1)
	{
		perror("Listen");
		exit(1); //TODO: return vals
	}
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
		    exit(1); //TODO: return vals
	}

	//Find the string representation of the given socket address
	if (inet_ntop(AF_INET, &(((struct sockaddr_in *)&new_address)->sin_addr), ipv4, INET_ADDRSTRLEN) == NULL)
	{
		    perror("Network to printable");
		    exit(1); //TODO: return vals
	}
	
	return new_fd;
}

/* Add a user with a given fd and sockaddr to the list of users */
void add_user(int fd, struct sockaddr_storage *address, struct user **users)
{
	struct user *temp;
	struct user *userptr;
    
    //Allocate memory for the new user
    temp = (struct user *)malloc(sizeof(struct user));
    
    //Set the default information for this new user
    temp->fd = fd;
    temp->name = NULL;
    temp->addr = *((struct sockaddr *)address);
    temp->next = NULL;
    
    //If this is the first user being added, the head will be NULL, so account for that
    if (*users == NULL)
    {
    	//The user being added is the only user currently on the server
    	*users = temp;
    }
    else
    {
		//Go to the end of the linked list
		for (userptr = *users; userptr->next != NULL; userptr = userptr->next);
		
		//Add the new user to the list of users
		userptr->next = temp;
	}
}

/* Find the user associated with the given fd */
struct user *get_user(struct user *users, int fd)
{
	struct user *userptr = users;
	
	//Iterate through the user list to find who is associated with the given fd
	while (userptr != NULL && userptr->fd != fd)
		userptr = userptr->next;
	
	//Return a pointer to the user who sent the message
	return userptr;
}

/* Handle the message that was received */
void handle_message(struct user **users, struct user *sender, struct cJSON *recvJSON, fd_set *master)
{
	//This char array will hold the ipv4 printable version of the user's IP
	char ipv4[INET_ADDRSTRLEN];

	//If recvJSON is null, the user has quit unexpectedly
	if (recvJSON == NULL)
    {
    	//If the sender had already given a name, tell the other users that this person has left
        if (sender->name != NULL)
        {
        	//Get the user left message
        	char *send_msg = generate_user_left_message(sender);
            
            //Send the message to all clients
            send_to_all(*users, send_msg, sender);
        }
        
        //Remove the user from the users list
        FD_CLR(sender->fd, master);
        remove_user(users, sender);
        return;
    }
    
    // If the user has not been initialized yet, fill in the user's data
    if (sender->name == NULL)
    {
    	char *name = cJSON_GetObjectItem(recvJSON, "from")->valuestring;
    	
    	//If that username is already in use, tell the user
    	if (username_already_in_use(*users, name))
    		send_invalid_username_message(sender);
    	//Otherwise, initialize the user with that name
    	else
    		initialize_user(sender, name, *users);
    	
		return;
    }
    
    // Find out what message the user sent
    char *msg = cJSON_GetObjectItem(recvJSON, "msg")->valuestring;
    
	//If the message is blank, don't allow it to be sent
	if (msg[0] == '\0')
		return;
    
    // If the user typed !quit, the user has left.
    if (!strcmp("!quit", msg))
    {
    	 //Get the user left message
		char *send_msg = generate_user_left_message(sender);
	
		//Send the message to all clients
		send_to_all(*users, send_msg, sender);
            
        //Remove the user from the users list
        FD_CLR(sender->fd, master);
        remove_user(users, sender);
        return;
    }
    
    // If the user is requesting a list of current users, build and send one.
    if (!strcmp("!who", msg))
    {
    	send_who_list(*users, sender);
    	return;
    }
    
    //If the user is asking for the help text, send it
    if (!strcmp("!help", msg))
    {
    	send_help_text(sender);
    	return;
    }
    
    // Default case; user sent a message which needs relayed to other users.
    if (inet_ntop(AF_INET, &(((struct sockaddr_in *)&(sender->addr))->sin_addr), ipv4, INET_ADDRSTRLEN) == NULL)
    {
            perror("Network to printable");
            exit(1); //TODO: Return vals
	}
    
    // Print the user's chat to the server's console
    printf("%s: %s\n", sender->name, msg);
    
    //Get the string representation of the JSON object
    char *send_msg = cJSON_Print(recvJSON);
	
    // Send message to other users
    send_to_all(*users, send_msg, sender);
}

/* Generate text saying that the given user has left the chat room */
char *generate_user_left_message(struct user *sender)
{
	//Create a cJSON object to send a message to all clients
	cJSON *sendJSON = cJSON_CreateObject();
	
	//Print to standard output that the user has left the chat
    fprintf(stderr, "%s has left the chat.\n", sender->name);
    
    //Allocate space for the leaveText
    char leaveText[strlen(" has left the chat.") + strlen(sender->name) + 1];
    
    //Print the message into the leaveText string
    sprintf(leaveText, "%s has left the chat.", sender->name);
    
    //Fill in the JSON data
    cJSON_AddStringToObject(sendJSON, "from", "SERVER");
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(leaveText));
    cJSON_AddStringToObject(sendJSON, "msg", leaveText);
    
    //Get the JSON data in string format
    char *send_msg = cJSON_Print(sendJSON);
    
    //Delete the cJSON object
    cJSON_Delete(sendJSON);
    
    //Return the leave text in JSON format
    return send_msg;
}

/* Send help text to the given user */
void send_help_text(struct user *user)
{
	//Create a cJSON object to hold the data
	cJSON *sendJSON = cJSON_CreateObject();
	
	//The standard help text
	static char *helptext = "\nType !quit to exit the chat.\nType !who to get a list of users.\nType !help to display this message again.\n";
    
    //Fill in the JSON data
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(helptext));
    cJSON_AddStringToObject(sendJSON, "msg", helptext);
    cJSON_AddStringToObject(sendJSON, "from", "SERVER");
    cJSON_AddNumberToObject(sendJSON, "valid", TRUE);
    
    //Get the JSON data in string format
    char *send_msg = cJSON_Print(sendJSON);
    
    //Delete the cJSON object
    cJSON_Delete(sendJSON);
    
    //Send the help text to the user
    send_to_user(send_msg, user);    
}

/* Initialize the new user's information and send a message that the user has entered the chat room */
void initialize_user(struct user *new_user, char *name, struct user *users)
{
	//Create a cJSON object to store the data
	cJSON *sendJSON = cJSON_CreateObject();
	
	//Properly set the new user's name
	new_user->name = strdup(name);
	
	//Send help text to the new user
	send_help_text(new_user);
	
	//Allocate space for the welcome message ("<name> has joined the chat.")
	char welcome_msg[strlen(name) + strlen("has joined the chat.") + 1];
	
	//Print "<name> has joined the chat." to the welcome_msg string
	sprintf(welcome_msg, "%s has joined the chat.", name);
	
	//Print to the server console that a new user has joined
	printf("%s\n", welcome_msg);
	
	//Add the data to the sendJSON
	cJSON_AddStringToObject(sendJSON, "from", "SERVER");
	cJSON_AddNumberToObject(sendJSON, "mlen", strlen(welcome_msg));
	cJSON_AddStringToObject(sendJSON, "msg", welcome_msg);
	
	//Print the JSON object to a string
	char *send_msg = cJSON_Print(sendJSON);
	
	//Send the welcome message to all users
	send_to_all(users, send_msg, new_user);
}

/* Find out if a given username is already in use in the linked list of users */
bool username_already_in_use(struct user *users, char *name)
{
	struct user *iter = users;
	
	//Iterate through the linked list
	while (iter != NULL)
	{
		//If the username is already in use, return TRUE
		if (iter->name != NULL && !strcmp(iter->name, name))
			return TRUE;
		
		//Increment the pointer
		iter = iter->next;
	}
	
	//If the username was never found, return FALSE
	return FALSE;
}

/* Tell the given user that the username they have chosen is already in use on the server */
void send_invalid_username_message(struct user *user)
{
	//Create a cJSON object to send a message to all clients
	cJSON *sendJSON = cJSON_CreateObject();
    
    char *msg = "Sorry, that username is already in use.";
    
    //Fill in the JSON data
    cJSON_AddStringToObject(sendJSON, "from", "SERVER");
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(msg));
    cJSON_AddStringToObject(sendJSON, "msg", msg);
    cJSON_AddNumberToObject(sendJSON, "valid", FALSE);
    
    //Find the string representation of the JSON object
    char *send_msg = cJSON_Print(sendJSON);
    
    //Send the message to the user
    send_to_user(send_msg, user);
    
    //Delete the JSON object
    cJSON_Delete(sendJSON);
}

/* Send a list of currently connected users to the given user */
void send_who_list(struct user *all_users, struct user *requester)
{
    char whotext[MAXMSG] = "Active users: ";
    struct user *temp = all_users;
    
    //Create a cJSON object to hold the information
    cJSON *sendJSON = cJSON_CreateObject();
    
    //Build a list of connected users for the given requesterr
    while (temp->next != NULL)
    {
        if (temp->name != NULL)
        {
                strcat(whotext, temp->name);
                strcat(whotext, ", ");
        }
        temp = temp->next;
    }
    //Append the last person's information to the who text
    if (temp->name != NULL)
	    strcat(whotext, temp->name);
    
    //Fill in the JSON data
    cJSON_AddStringToObject(sendJSON, "from", "SERVER");
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(whotext));
    cJSON_AddStringToObject(sendJSON, "msg", whotext);
    
    //Get the string representation of the JSON object
    char *send_msg = cJSON_Print(sendJSON);
    
    //Send the message to the specified user
   	send_to_user(send_msg, requester);
   	
   	//Delete the cJSON Object
   	cJSON_Delete(sendJSON);
}

/* Send a message to all users except for the user who initially sent the message */
void send_to_all(struct user *users, char *send_msg, struct user *sender)
{
	//Iterate through the users list
	struct user *iter = users;
	while (iter != NULL)
	{
		//If the current user is not the sender,
		if (iter != sender)
		{
			//Send the message to that user
			if (send(iter->fd, send_msg, strlen(send_msg), 0) == -1)
			{
				perror("Send");
				exit(1); //TODO: Return vals
			}
		}
		
		//Increment the iterator
		iter = iter->next;
	}
}

/* Send a message to the specified user */
void send_to_user(char *send_msg, struct user *user)
{
	if (send(user->fd, send_msg, strlen(send_msg), 0) == -1)
		perror("Send");
}

/* Remove a user from a linked list of users */
void remove_user(struct user **users, struct user *user_to_remove)
{
	//If the user is at the head of the users linked list, update it accordingly
	if (*users == user_to_remove)
	{
		*users = user_to_remove->next;
	}
	else
	{
		//Otherwise, iteratively go through the users list to remove the user
		struct user *iter = *users;
		while (iter->next != user_to_remove)
			iter = iter->next;
			
		//Update the next pointer to skip over the user to be removed
		iter->next = iter->next->next;
	}
	
	//Close the fd pointing to the removed user and free the memory
	close(user_to_remove->fd);
	free(user_to_remove);
	free(user_to_remove->name);
}
