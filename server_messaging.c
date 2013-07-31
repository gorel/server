#include "server_messaging.h"

/* Handle the message that was received */
void handle_message(struct user **users, struct user *sender, struct cJSON *recvJSON, fd_set *master)
{
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
    
    //The user wants to send a private message
    if (!strncmp("!tell", msg, strlen("!tell")))
    {
    	//The target user to send the message to
    	struct user *target;
    	char *name;
    	
    	//We know the first word is !tell, so extract it with the string tokenizer
    	strtok(msg, " ");
    	
    	//Find the target user with the string tokenizer
    	name = strtok(NULL, " ");
    	target = get_user_by_name(*users, name);
    	
    	//If the user was found, send them the message
    	if (target != NULL)
    	{
    		//Extract the rest of the message and send it to the target
    		msg = strtok(NULL, "\0");
    		send_private_message(sender->name, msg, target);
    	}
    	//Otherwise, tell the sender that their desired recipient could not be found
    	else
    		send_user_not_found_message(sender);
    		
    	return;
    }
    
    // Print the user's chat to the server's console
    printf("%s: %s\n", sender->name, msg);
    
    //The message is not private, so add this to the JSON object
    cJSON_AddNumberToObject(recvJSON, "private", FALSE);
    
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
    cJSON_AddNumberToObject(sendJSON, "private", FALSE);
    
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
	static char *helptext = "\nType !quit to exit the chat.\nType !who to get a list of users.\nType !tell <user> <message> to send a private message.\nType !help to display this message again.\n";
    
    //Fill in the JSON data
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(helptext));
    cJSON_AddStringToObject(sendJSON, "msg", helptext);
    cJSON_AddStringToObject(sendJSON, "from", "SERVER");
    cJSON_AddNumberToObject(sendJSON, "valid", TRUE);		//Needed when sending the initial message
    cJSON_AddNumberToObject(sendJSON, "private", FALSE);
    
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
    cJSON_AddNumberToObject(sendJSON, "private", FALSE);
	
	//Print the JSON object to a string
	char *send_msg = cJSON_Print(sendJSON);
	
	//Send the welcome message to all users
	send_to_all(users, send_msg, new_user);
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
    cJSON_AddNumberToObject(sendJSON, "private", FALSE);
    
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
    cJSON_AddNumberToObject(sendJSON, "private", FALSE);
    
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

/* Send a private message to the given user */
void send_private_message(char *from, char *msg, struct user *user)
{
	//Create a cJSON object to hold the information
    cJSON *sendJSON = cJSON_CreateObject();
    
    //Fill in the JSON data
    cJSON_AddStringToObject(sendJSON, "from", from);
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(msg));
    cJSON_AddStringToObject(sendJSON, "msg", msg);
    cJSON_AddNumberToObject(sendJSON, "private", TRUE);
    
    //Get the string representation of the JSON object
    char *send_msg = cJSON_Print(sendJSON);
    
    //Print the message to the server's console
    printf("[Private] %s to %s: %s\n", from, user->name, msg);
    
    //Send the message to the specified user
   	send_to_user(send_msg, user);
   	
   	//Delete the cJSON Object
   	cJSON_Delete(sendJSON);
}

/* Tell the given user that the name they supplied could not be found within the list of active users */
void send_user_not_found_message(struct user *from)
{
	//Create a cJSON object to hold the information
    cJSON *sendJSON = cJSON_CreateObject();
    
    char *msg = "No user found with that user name.";
    
    //Fill in the JSON data
    cJSON_AddStringToObject(sendJSON, "from", "SERVER");
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(msg));
    cJSON_AddStringToObject(sendJSON, "msg", msg);
    cJSON_AddNumberToObject(sendJSON, "private", FALSE);
    
    //Get the string representation of the JSON object
    char *send_msg = cJSON_Print(sendJSON);
    
    //Send the message to the specified user
   	send_to_user(send_msg, from);
   	
   	//Delete the cJSON Object
   	cJSON_Delete(sendJSON);
}

/* Send a message to the specified user */
void send_to_user(char *send_msg, struct user *user)
{
	if (send(user->fd, send_msg, strlen(send_msg), 0) == -1)
		perror("Send");
}
