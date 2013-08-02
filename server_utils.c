#include "server_utils.h"
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
	
	//Accept the new connection
	if ((new_fd = accept(listen_fd, (struct sockaddr *)new_address, &addrLen)) == -1)
	{
		    perror("Accept");
		    exit(1); //TODO: return vals
	}
	
	return new_fd;
}

/* Add a user with a given fd and sockaddr to the list of users */
void add_user(int fd, struct sockaddr_storage *address, struct user **users)
{
	struct user *temp;
	struct user *iter;
    
    //Allocate memory for the new user
    temp = (struct user *)malloc(sizeof(struct user));
    
    //Set the default information for this new user
    temp->fd = fd;
    temp->name = NULL;
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
		for (iter = *users; iter->next != NULL; iter = iter->next);
		
		//Add the new user to the list of users
		iter->next = temp;
	}
}

/* Find the user associated with the given fd */
struct user *get_user_by_fd(struct user *users, int fd)
{
	struct user *iter = users;
	
	//Iterate through the user list to find who is associated with the given fd
	while (iter != NULL && iter->fd != fd)
		iter = iter->next;
	
	//Return a pointer to the user who sent the message
	return iter;
}

/* Find the user associated with the given name -- may return NULL if user with that name not found */
struct user *get_user_by_name(struct user *users, char *name)
{
	//If the name is NULL, don't even bother checking
	if (name == NULL)
		return NULL;

	struct user *iter = users;
	
	//Iterate through the user list to find who is associated with the given name
	while (iter != NULL && strcmp(iter->name, name))
		iter = iter->next;
		
	//Return a pointer to the user who sent the message
	//NOTE: Returns NULL if the user was not found
	return iter;
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
	free(user_to_remove->name);
	free(user_to_remove);
}

/* Update the user's ignore list to include the new user */
void ignore(struct user *user, struct user *user_to_ignore)
{
	int i;

	//If the user is already ignoring the user, don't let them add the person to the list again
	if (ignoring(user, user_to_ignore))
		return;
	
	//Iterate through the user's ignore list
	for (i = 0; i < MAXIGNORE; i++)
		//If ignore_list[i] is NULL, the slot is open -- set it to user_to_ignore and return
		if (user->ignore_list[i] == NULL)
		{
			user->ignore_list[i] = user_to_ignore;
			return;
		}
}

/* Update the user's ignore list to remove the given user */
void unignore(struct user *user, struct user *user_to_unignore)
{
	int i;
	
	//Iterate through the user's ignore list
	for (i = 0; i < MAXIGNORE; i++)
		//If the target is found, set ignore_list[i] to NULL and return
		if (user->ignore_list[i] == user_to_unignore)
		{
			user->ignore_list[i] = NULL;
			return;
		}
}

/* Return whether or not the given user is ignoring <target> */
bool ignoring(struct user *user, struct user *target)
{
	int i;
	
	//Iterate through the user's ignore list
	for (i = 0; i < MAXIGNORE; i++)
		//If the target is found, return TRUE
		if (user->ignore_list[i] == target)
			return TRUE;
	
	//The target was never found; return FALSE
	return FALSE;
}
