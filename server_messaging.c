#include "server_messaging.h"

/* Generate text saying that the given user has left the chat room */
void send_user_left_message(struct user *users, struct user *sender)
{
	//Print to standard output that the user has left the chat
    printf("%s has left the chat.\n", sender->name);
    
    //Allocate space for the leaveText
    char leave_text[strlen(" has left the chat.") + strlen(sender->name) + 1];
    
    //Print the message into the leaveText string
    sprintf(leave_text, "%s has left the chat.", sender->name);
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", leave_text, TRUE, FALSE, FALSE);
    
    //Send the leave message to all users
    send_to_all(users, send_msg, sender);
    
    //Free send_msg
    free(send_msg);
}

/* Send help text to the given user */
void send_help_text(struct user *user)
{
	//The standard help text
	static char *help_text = "\n!admins :\t\t\tGet a list of currently online admins\
							 \n!adminhelp :\t\t\tGet a list of admin commands (ADMIN ONLY)\
							 \n!afk :\t\t\t\tSwitch your away from keyboard status\
							 \n!ignore <user> :\t\tStop receiving private messages from <user>\
							 \n!unignore <user> :\t\tAllow <user> to send you private messages\
							 \n!tell <user> <message> :\tSend a private message to <user>\
							 \n!who :\t\t\t\tGet a list of currently online users\
							 \n!help :\t\t\t\tDisplay this information again\
							 \n";
    
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", help_text, TRUE, FALSE, FALSE);
    
    //Send the help text to the user
    send_to_user(send_msg, user);
    
    //Free send_msg
    free(send_msg);
}


/* Send the user their current AFK status */
void send_afk_status(struct user *user)
{
	char *afk_msg = NULL;
    
    //Set the afk_msg by whether or not the user is afk
    if (user->afk)
    	afk_msg = "You are now away from keyboard.";
    else
    	afk_msg = "You are no longer away from keyboard.";
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", afk_msg, TRUE, FALSE, FALSE);
    
    //Send the message to the user
    send_to_user(send_msg, user);
    
    //Free send_msg
    free(send_msg);
}

/* Initialize the new user's information and send a message that the user has entered the chat room */
void initialize_user(struct user *new_user, char *name, struct user *users)
{
	int i;
	
	//Properly set the new user's name
	new_user->name = strdup(name);
	
	//Make sure all entries in the user's ignore list are NULL
	for (i = 0; i < MAXIGNORE; i++)
		new_user->ignore_list[i] = NULL;
	
	//New users should not be muted or AFK by default or have admin privileges (unless the user is named ADMIN)
	new_user->muted = FALSE;
	new_user->afk = FALSE;
	if (!strcmp("ADMIN", new_user->name))
		new_user->admin = TRUE;
	else
		new_user->admin = FALSE;
	
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
	
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", welcome_msg, TRUE, FALSE, FALSE);
	
	//Send the welcome message to all users
	send_to_all(users, send_msg, new_user);
	
	//Free send_msg
	free(send_msg);
}

/* Tell the given user that the username they have chosen is already in use on the server */
void send_invalid_username_message(struct user *user)
{
	static char *msg = "Sorry, that username is already in use.";
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", msg, FALSE, FALSE, FALSE);
    
    //Send the message to the user
    send_to_user(send_msg, user);
    
    //Free send_msg
    free(send_msg);
}

/* Send a list of currently connected users to the given user */
void send_who_list(struct user *all_users, struct user *requester)
{
    char who_text[MAXMSG] = "Active users: ";
    struct user *temp = all_users;
    
    //Build a list of connected users for the given requester
    while (temp->next != NULL)
    {
        if (temp->name != NULL)
        {
                strcat(who_text, temp->name);
                strcat(who_text, " ");
        }
        temp = temp->next;
    }
    //Append the last person's information to the who text
    if (temp->name != NULL)
	    strcat(who_text, temp->name);
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", who_text, TRUE, FALSE, FALSE);
    
    //Send the message to the specified user
   	send_to_user(send_msg, requester);
   	
   	//Free send_msg
   	free(send_msg);
}

/* Send a list of currently connected admins to the given user */
void send_admin_list(struct user *all_users, struct user *requester)
{
	char who_text[MAXMSG] = "Active admins: ";
	bool any = FALSE;
	
    struct user *temp = all_users;
    
    //Build a list of connected admins for the given requester
    while (temp->next != NULL)
    {
        if (temp->name != NULL && temp->admin)
        {
                strcat(who_text, temp->name);
                strcat(who_text, " ");
                any = TRUE;
        }
        temp = temp->next;
    }
    //Append the last person's information to the who text
    if (temp->name != NULL && temp->admin)
    {
	    strcat(who_text, temp->name);
	    any = TRUE;
	}
	    
	//If there are no active admins, append "<none>" to the who text
	if (!any)
		strcat(who_text, "<none>");
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", who_text, TRUE, FALSE, FALSE);
    
    //Send the message to the specified user
   	send_to_user(send_msg, requester);
   	
   	//Free send_msg
   	free(send_msg);
}

/* Warn a user that they tried to send a private message to a user listed as AFK */
void send_afk_warning(struct user *user)
{
	static char *msg = "Warning: Target user is AFK.  They might not see your message.";
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", msg, TRUE, FALSE, FALSE);
    
    //Send the message to the specified user
   	send_to_user(send_msg, user);
   	
   	//Free send_msg
   	free(send_msg);
}

/* Send a private message to the given user */
void send_private_message(char *from, char *msg, struct user *user)
{
	//Get a JSON-formatted string
    char *send_msg = allocate_json_string(from, msg, TRUE, TRUE, FALSE);
    
    //Print the message to the server's console
    printf("[Private] %s to %s: %s\n", from, user->name, msg);
    
    //Send the message to the specified user
   	send_to_user(send_msg, user);
   	
   	//Free  send_msg
   	free(send_msg);
}

/* Send a message to the given user telling them that they are currently muted */
void send_you_are_muted_message(struct user *user)
{
	static char *muted_msg = "Cannot send: you are currently muted.";
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", muted_msg, TRUE, FALSE, FALSE);
    
    //Send the message to the specified user
   	send_to_user(send_msg, user);
   	
   	//Free send_msg
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
	static char *msg = "No user found with that user name.";
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", msg, TRUE, FALSE, FALSE);
    
    //Send the message to the specified user
   	send_to_user(send_msg, from);
   	
   	//Free send_msg
   	free(send_msg);
}

/* Send a message to the specified user */
void send_to_user(char *send_msg, struct user *user)
{
	if (send(user->fd, send_msg, strlen(send_msg), 0) == -1)
		perror("Send");
}

/* Send a message to the given user that they are now ignoring someone new */
void send_ignore_message(struct user *user, char *ignoring)
{
	char msg[strlen("You are now ignoring ") + strlen(ignoring) + 1];
    
    //Print "You are no longer ignoring <user>" to the msg string
    sprintf(msg, "You are now ignoring %s", ignoring);
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", msg, TRUE, FALSE, FALSE);
    
    //Send the message to the user
    send_to_user(send_msg, user);
    
    //Free send_msg
    free(send_msg);
}

/* Send a message to the given user that they are no longer ignoring the given target */
void send_unignore_message(struct user *user, char *unignoring)
{
	//Allocate space for the message
    char msg[strlen("You are no longer ignoring ") + strlen(unignoring) + 1];
    
    //Print "You are no longer ignoring <user>" to the msg string
    sprintf(msg, "You are no longer ignoring %s", unignoring);
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", msg, TRUE, FALSE, FALSE);
    
    //Send the message to the user
    send_to_user(send_msg, user);
    
    //Free send_msg
    free(send_msg);
}

/* Send a message to the given user that they are being ignored */
void send_you_are_ignored_message(struct user *user, char *ignorer_name)
{
	//Allocate space for the message
    char msg[strlen(ignorer_name) + strlen(" is ignoring you") + 1];
    
    //Print "You are no longer ignoring <user>" to the msg string
    sprintf(msg, "%s is ignoring you", ignorer_name);
    
    //Get a JSON-formatted string
    char *send_msg = allocate_json_string("SERVER", msg, TRUE, FALSE, FALSE);
    
    //Send the message to the user
    send_to_user(send_msg, user);
    
    //Free send_msg
    free(send_msg);
}
