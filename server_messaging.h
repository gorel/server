#ifndef SERVER_MESSAGING_H
#define SERVER_MESSAGING_H

#include "server_utils.h"
#include "admin.h"

/* Generate text saying that the given user has left the chat room */
void send_user_left_message(struct user *users, struct user *sender);

/* Send help text to the given user */
void send_help_text(struct user *user);

/* Send the user their current AFK status */
void send_afk_status(struct user *user);

/* Initialize the new user's information and send a message that the user has entered the chat room */
void initialize_user(struct user *new_user, char *name, struct user *users);

/* Tell the given user that the username they have chosen is already in use on the server */
void send_invalid_username_message(struct user *user);

/* Send a list of currently connected users to the given user */
void send_who_list(struct user *all_users, struct user *requester);

/* Send a message to all users except for the user who initially sent the message */
void send_to_all(struct user *users, char *send_msg, struct user *sender);

/* Send a list of currently connected admins to the given user */
void send_admin_list(struct user *all_users, struct user *requester);

/* Warn a user that they tried to send a private message to a user listed as AFK */
void send_afk_warning(struct user *user);

/* Send a private message to the given user */
void send_private_message(char *from, char *msg, struct user *user);

/* Send a message to the given user telling them that they are currently muted */
void send_you_are_muted_message(struct user *user);

/* Tell the given user that the name they supplied could not be found within the list of active users */
void send_user_not_found_message(struct user *from);

/* Send a message to the specified user */
void send_to_user(char *send_msg, struct user *user);

/* Send a message to the given user that they are now ignoring someone new */
void send_ignore_message(struct user *user, char *ignoring);

/* Send a message to the given user that they are no longer ignoring the given target */
void send_unignore_message(struct user *user, char *unignoring);

/* Send a message to the given user that they are being ignored */
void send_you_are_ignored_message(struct user *user, char *ignorer_name);
    			
#endif
