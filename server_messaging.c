#include "server_messaging.h"

/* Generate text saying that the given user has left the chat room */
void send_user_left_message(struct user *users, struct user *sender)
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
    
    //Send the leave message to all users
    send_to_all(users, send_msg, sender);
    
    //Delete the cJSON object and free send_msg
    cJSON_Delete(sendJSON);
    free(send_msg);
}

/* Send help text to the given user */
void send_help_text(struct user *user)
{
	//Create a cJSON object to hold the data
	cJSON *sendJSON = cJSON_CreateObject();
	
	//The standard help text
	static char *helptext = "\nType !quit to exit the chat.\nType !who to get a list of users.\nType !tell <user> <message> to send a private message.\nType !help to display this message again.\nType !admins for a list of current admins online\nType !adminhelp for a list of admin commands\n";
    
    //Fill in the JSON data
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(helptext));
    cJSON_AddStringToObject(sendJSON, "msg", helptext);
    cJSON_AddStringToObject(sendJSON, "from", "SERVER");
    cJSON_AddNumberToObject(sendJSON, "valid", TRUE);		//Needed when sending the initial message
    cJSON_AddNumberToObject(sendJSON, "private", FALSE);
    
    //Get the JSON data in string format
    char *send_msg = cJSON_Print(sendJSON);
    
    //Send the help text to the user
    send_to_user(send_msg, user);
    
    //Delete the cJSON object and the send_msg
    cJSON_Delete(sendJSON);
    free(send_msg);
}

/* Initialize the new user's information and send a message that the user has entered the chat room */
void initialize_user(struct user *new_user, char *name, struct user *users)
{
	//Create a cJSON object to store the data
	cJSON *sendJSON = cJSON_CreateObject();
	
	//Properly set the new user's name
	new_user->name = strdup(name);
	
	//New users should not have admin privileges or be muted by default (unless the user is named ADMIN)
	if (!strcmp("ADMIN", new_user->name))
		new_user->admin = TRUE;
	else
		new_user->admin = FALSE;
	new_user->muted = FALSE;
	
	//Send help text to the new user
	send_help_text(new_user);
	
	//If the user is an admin, send the admin help text
	if (new_user->admin)
		send_admin_help_text(new_user);
	
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
	
	//Delete the cJSON object and free send_msg
	cJSON_Delete(sendJSON);
	free(send_msg);
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
    
    //Delete the JSON object and free send_msg
    cJSON_Delete(sendJSON);
    free(send_msg);
}

/* Send a list of currently connected users to the given user */
void send_who_list(struct user *all_users, struct user *requester)
{
    char whotext[MAXMSG] = "Active users: ";
    struct user *temp = all_users;
    
    //Create a cJSON object to hold the information
    cJSON *sendJSON = cJSON_CreateObject();
    
    //Build a list of connected users for the given requester
    while (temp->next != NULL)
    {
        if (temp->name != NULL)
        {
                strcat(whotext, temp->name);
                strcat(whotext, " ");
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
   	
   	//Delete the cJSON Object and free send_msg
   	cJSON_Delete(sendJSON);
   	free(send_msg);
}

/* Send a list of currently connected admins to the given user */
void send_admin_list(struct user *all_users, struct user *requester)
{
	char whotext[MAXMSG] = "Active admins: ";
	bool any = FALSE;
	
    struct user *temp = all_users;
    
    //Create a cJSON object to hold the information
    cJSON *sendJSON = cJSON_CreateObject();
    
    //Build a list of connected admins for the given requester
    while (temp->next != NULL)
    {
        if (temp->name != NULL && temp->admin)
        {
                strcat(whotext, temp->name);
                strcat(whotext, " ");
                any = TRUE;
        }
        temp = temp->next;
    }
    //Append the last person's information to the who text
    if (temp->name != NULL && temp->admin)
    {
	    strcat(whotext, temp->name);
	    any = TRUE;
	}
	    
	//If there are no active admins, append "<none>" to the who text
	if (!any)
		strcat(whotext, "<none>");
    
    //Fill in the JSON data
    cJSON_AddStringToObject(sendJSON, "from", "SERVER");
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(whotext));
    cJSON_AddStringToObject(sendJSON, "msg", whotext);
    cJSON_AddNumberToObject(sendJSON, "private", FALSE);
    
    //Get the string representation of the JSON object
    char *send_msg = cJSON_Print(sendJSON);
    
    //Send the message to the specified user
   	send_to_user(send_msg, requester);
   	
   	//Delete the cJSON Object and free send_msg
   	cJSON_Delete(sendJSON);
   	free(send_msg);
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
   	
   	//Delete the cJSON object and free send_msg
   	cJSON_Delete(sendJSON);
   	free(send_msg);
}

/* Send a message to the given user telling them that they are currently muted */
void send_you_are_muted_message(struct user *user)
{
	//Create a cJSON object to hold the information
    cJSON *sendJSON = cJSON_CreateObject();
    
    char *muted_msg = "Cannot send: you are currently muted.";
    
    //Fill in the JSON data
    cJSON_AddStringToObject(sendJSON, "from", "SERVER");
    cJSON_AddNumberToObject(sendJSON, "mlen", strlen(muted_msg));
    cJSON_AddStringToObject(sendJSON, "msg", muted_msg);
    cJSON_AddNumberToObject(sendJSON, "private", TRUE);
    
    //Get the string representation of the JSON object
    char *send_msg = cJSON_Print(sendJSON);
    
    //Send the message to the specified user
   	send_to_user(send_msg, user);
   	
   	//Delete the cJSON object and free send_msg
   	cJSON_Delete(sendJSON);
   	free(send_msg);
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
   	
   	//Delete the cJSON Object and free send_msg
   	cJSON_Delete(sendJSON);
   	free(send_msg);
}

/* Send a message to the specified user */
void send_to_user(char *send_msg, struct user *user)
{
	if (send(user->fd, send_msg, strlen(send_msg), 0) == -1)
		perror("Send");
}
