#include "handle_message.h"
#include "admin.h"

/* Handle the message that was received */
void handle_message(struct user **users, struct user *sender, struct cJSON *recvJSON, fd_set *master)
{
	//If recvJSON is null, the user has quit unexpectedly
	if (recvJSON == NULL)
    {
    	//If the sender had already given a name, tell the other users that this person has left
        if (sender->name != NULL)
        	//Send the user left message
        	send_user_left_message(*users, sender);
        
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
		
	//Handle the case in which an admin action was called
	bool admin_action_called = handle_admin_action(users, master, sender, msg);
	
	//If the user performed an admin action, return
	if (admin_action_called)
		return;
    
    // If the user typed !quit, the user has left.
    if (!strncmp("!quit", msg, strlen("!quit ")))
    {
    	//Send the user left message
		send_user_left_message(*users, sender);
            
        //Remove the user from the users list
        FD_CLR(sender->fd, master);
        remove_user(users, sender);
        return;
    }
    
    // If the user is requesting a list of current users, build and send one.
    if (!strncmp("!who", msg, strlen("!who ")))
    {
    	send_who_list(*users, sender);
    	return;
    }
    
    //If the user is asking for the help text, send it
    if (!strncmp("!help", msg, strlen("!help ")))
    {
    	send_help_text(sender);
    	return;
    }
    
    //If the user is asking for a list of the currently online admins, send it
    if (!strncmp("!admins", msg, strlen("!admins ")))
    {
    	send_admin_list(*users, sender);
    	return;
    }
    
    //The user wants to send a private message
    if (!strncmp("!tell ", msg, strlen("!tell ")))
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
    		//If the user is currently muted and they aren't trying to talk to an admin, don't let the message go through
    		if (sender->muted && !target->admin)
    		{
    			send_you_are_muted_message(sender);
    			return;
    		}
    		
    		//Extract the rest of the message and send it to the target
    		msg = strtok(NULL, "\0");
    		send_private_message(sender->name, msg, target);
    	}
    	//Otherwise, tell the sender that their desired recipient could not be found
    	else
    		send_user_not_found_message(sender);
    		
    	return;
    }
    
    //If the user is currently muted, don't let the message go through
	if (sender->muted)
	{
		send_you_are_muted_message(sender);
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
