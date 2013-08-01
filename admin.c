#include "admin.h"

/* This will be called within handle_message -- find out if the user is performing an admin action and act accordingly
 * Return TRUE if the user was performing an admin action, FALSE otherwise
 */
bool handle_admin_action(struct user **users, fd_set *master, struct user *sender, char *msg)
{
	//Duplicate message so the original won't be modified
	char *new_msg = strdup(msg);
	
	//Extract the action with a string tokenizer
	char *action = strtok(new_msg, " ");
	
	//If the sender is trying to promote someone to an admin, find the target and call promote_to_admin
	if (!strncmp("!admin", action, strlen("!admin ")))
	{
		//Find the target user with the string tokenizer
		char *name = strtok(NULL, " ");
		struct user *target = get_user_by_name(*users, name);
		
		//Try to promote the given user to an admin and return TRUE
		promote_to_admin(*users, sender, target);
		return TRUE;
	}
	
	//If the sender is trying to mute someone, find the target and call mute
	else if (!strncmp("!mute", action, strlen("!mute ")))
	{
		//Find the target user with the string tokenizer
		char *name = strtok(NULL, " ");
		struct user *target = get_user_by_name(*users, name);
		
		//Try to mute the given user and return TRUE
		mute(sender, target);
		return TRUE;
	}
	
	//If the sender is trying to unmute someone, find the target and call unmute
	else if (!strncmp("!unmute", action, strlen("!unmute ")))
	{
		//Find the target user with the string tokenizer
		char *name = strtok(NULL, " ");
		struct user *target = get_user_by_name(*users, name);
		
		//Try to unmute the given user and return TRUE
		unmute(sender, target);
		return TRUE;
	}
	
	//If the sender is trying to kick someone, find the target and call kick
	else if (!strncmp("!kick", action, strlen("!kick ")))
	{
		//Find the target user with the string tokenizer
		char *name = strtok(NULL, " ");
		struct user *target = get_user_by_name(*users, name);
		char *reason = strtok(NULL, "\0");
		
		//If no reason was given, set the reason as "(no reason given)"
		if (reason == NULL)
			reason = "(no reason given)";
		
		//Try to kick the given user and return TRUE
		kick(users, master, sender, target, reason);
		return TRUE;
	}
	
	//The sender is not performing an admin action.  Return FALSE
	return FALSE;
}

/* Send admin help text to the given user (will check that user is an admin) */
void send_admin_help_text(struct user *user)
{
	if (user->admin)
	{
		//Create a cJSON object to hold the data
		cJSON *sendJSON = cJSON_CreateObject();
	
		//The standard admin help text
		static char *helptext = "\nType !admin <user> to give <user> admin privileges\nType !mute <user> to mute the given user\nType !unmute <user> to unmute the given user\nType !kick <user> to kick the given user\n";
		
		//Fill in the JSON data
		cJSON_AddNumberToObject(sendJSON, "mlen", strlen(helptext));
		cJSON_AddStringToObject(sendJSON, "msg", helptext);
		cJSON_AddStringToObject(sendJSON, "from", "SERVER");
		cJSON_AddNumberToObject(sendJSON, "private", FALSE);
		
		//Get the JSON data in string format
		char *send_msg = cJSON_Print(sendJSON);
		
		//Send the help text to the user
		send_to_user(send_msg, user);
		
		//Delete the cJSON object and the send_msg
		cJSON_Delete(sendJSON);
		free(send_msg);
	}
	//Otherwise, tell the user that they don't have privileges for that
	else
		send_not_admin_message(user);
}

/* Promote the specified user to an administrator (will check that admin has the correct privileges) */
void promote_to_admin(struct user *users, struct user *admin, struct user *user_to_promote)
{
	//If the selected user is NULL, send a user_not_found message and return
	if (user_to_promote == NULL)
	{
		send_user_not_found_message(admin);
		return;
	}
		
	//If "admin" has admin privileges, promote the given user to an admin
	if (admin->admin)
	{
		user_to_promote->admin = TRUE;
		
		//Send a message to all users that this user was promoted to an admin
		send_new_admin_message(users, user_to_promote->name);
		
	}
	//Otherwise, tell the user that they don't have privileges for that
	else
		send_not_admin_message(admin);
		
}

/* Send a message to all users that a new admin has been added to the server */
void send_new_admin_message(struct user *users, char *new_admin_name)
{
	//Create a cJSON object to store the data
	cJSON *sendJSON = cJSON_CreateObject();
	
	//Allocate space for the kick message ("<name> being kicked for <reason>")
	char admin_msg[strlen(new_admin_name) + strlen(" has been promoted to admin") + 1];

	//Print "<name> has been prmoted to admin" to the admin_msg string
	sprintf(admin_msg, "%s has been promoted to admin", new_admin_name);

	//Print to the server console that there is a new admin
	printf("%s\n", admin_msg);

	//Add the data to the sendJSON
	cJSON_AddStringToObject(sendJSON, "from", "SERVER");
	cJSON_AddNumberToObject(sendJSON, "mlen", strlen(admin_msg));
	cJSON_AddStringToObject(sendJSON, "msg", admin_msg);
	cJSON_AddNumberToObject(sendJSON, "private", FALSE);

	//Print the JSON object to a string
	char *send_msg = cJSON_Print(sendJSON);
	
	//Send a message to all users that there is a new admin
	send_to_all(users, send_msg, NULL);
	
	//Free the cJSON object and send_msg
	cJSON_Delete(sendJSON);
	free(send_msg);
}

/* Mute the given user (will check that admin has the correct privileges) */
void mute(struct user *admin, struct user *user_to_mute)
{
	//If the selected user is NULL, send a user_not_found message and return
	if (user_to_mute == NULL)
	{
		send_user_not_found_message(admin);
		return;
	}
	
	//If "admin" has admin privileges, mute the given user
	if (admin->admin)
		user_to_mute->muted = TRUE;
	//Otherwise, tell the user that they don't have privileges for that
	else
		send_not_admin_message(admin);
	
}

/* Unmute the given user (will check that admin has the correct privileges) */
void unmute(struct user *admin, struct user *user_to_unmute)
{
	//If the selected user is NULL, send a user_not_found message and return
	if (user_to_unmute == NULL)
	{
		send_user_not_found_message(admin);
		return;
	}
	
	//If "admin" has admin privileges, unmute the given user
	if (admin->admin)
		user_to_unmute->muted = FALSE;
	//Otherwise, tell the user that they don't have privileges for that
	else
		send_not_admin_message(admin);
}

/* Kick the given user out of the chat room for the given reason (will check that admin has the correct privileges) */
void kick(struct user **users, fd_set *master, struct user *admin, struct user *user_to_kick, char *reason)
{
	//If the selected user is NULL, send a user_not_found message and return
	if (user_to_kick == NULL)
	{
		send_user_not_found_message(admin);
		return;
	}
	
	//If "admin" has admin privileges, kick the given user
	if (admin->admin)
	{
		//Create a cJSON object to store the data
		cJSON *sendJSON = cJSON_CreateObject();
		
		//Allocate space for the kick message ("<name> being kicked for <reason>")
		char kick_msg[strlen(user_to_kick->name) + strlen(" being kicked for ") + strlen(reason) + 1];

		//Print "<name> being kicked for <reason>" to the kick_msg string
		sprintf(kick_msg, "%s being kicked for %s", user_to_kick->name, reason);

		//Print to the server console that a user is being kicked
		printf("%s\n", kick_msg);

		//Add the data to the sendJSON
		cJSON_AddStringToObject(sendJSON, "from", "SERVER");
		cJSON_AddNumberToObject(sendJSON, "mlen", strlen(kick_msg));
		cJSON_AddStringToObject(sendJSON, "msg", kick_msg);
		cJSON_AddNumberToObject(sendJSON, "private", FALSE);

		//Print the JSON object to a string
		char *send_msg = cJSON_Print(sendJSON);
		
		//Send a message to all users stating why user_to_kick is being removed
		send_to_all(*users, send_msg, NULL);
		
		//Remove the user from the linked list of users
        FD_CLR(user_to_kick->fd, master);
		remove_user(users, user_to_kick);
		
		//Free the cJSON object and send_msg
		cJSON_Delete(sendJSON);
		free(send_msg);
	}
	//Otherwise, tell the user that they don't have privileges for that
	else
		send_not_admin_message(admin);
}

/* Tell the specified user that they do not have admin privileges */
void send_not_admin_message(struct user *user)
{
	//Create a cJSON object to store the data
	cJSON *sendJSON = cJSON_CreateObject();
	
	//Define the error message ("Error: You must be an admin to do that.")
	char *error_msg = "Error: You must be an admin to do that.";
	
	//Add the data to the sendJSON object
	cJSON_AddStringToObject(sendJSON, "from", "SERVER");
	cJSON_AddNumberToObject(sendJSON, "mlen", strlen(error_msg));
	cJSON_AddStringToObject(sendJSON, "msg", error_msg);
	cJSON_AddNumberToObject(sendJSON, "private", FALSE);
	
	//Print the JSON object to a string
	char *send_msg = cJSON_Print(sendJSON);
	
	//Send the error message to the user trying to do whatever is requiring admin privileges
	send_to_user(send_msg, user);
	
	//Free the cJSON object and send_msg
	cJSON_Delete(sendJSON);
	free(send_msg);
}
