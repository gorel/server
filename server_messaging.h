#ifndef SERVER_MESSAGING_H
#define SERVER_MESSAGING_H

#include "server_utils.h"


/* Handle the message that was received */
void handle_message(struct user **users, struct user *sender, struct cJSON *recvJSON, fd_set *master);

/* Generate text saying that the given user has left the chat room */
char *generate_user_left_message(struct user *sender);

/* Send help text to the given user */
void send_help_text(struct user *user);

/* Initialize the new user's information and send a message that the user has entered the chat room */
void initialize_user(struct user *new_user, char *name, struct user *users);

/* Tell the given user that the username they have chosen is already in use on the server */
void send_invalid_username_message(struct user *user);

/* Send a list of currently connected users to the given user */
void send_who_list(struct user *all_users, struct user *requester);

/* Send a message to all users except for the user who initially sent the message */
void send_to_all(struct user *users, char *send_msg, struct user *sender);

/* Send a private message to the given user */
void send_private_message(char *from, char *msg, struct user *user);

/* Tell the given user that the name they supplied could not be found within the list of active users */
void send_user_not_found_message(struct user *from);

/* Send a message to the specified user */
void send_to_user(char *send_msg, struct user *user);


#endif
