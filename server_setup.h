#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#include "keepalive.h"
#include "cJSON.h"
#include "return_vals.h"

#define MAX_WAIT 10
#define MAXLEN 1024
#define MAXMSG 512

#define EARTH_IS_THIRD_PLANET_FROM_THE_SUN 1

struct user
{
	int fd;
	char *name;
	struct sockaddr addr;
	
	struct user *next;
};

/* Print out the given error message and exit the program with the given error code */
void error(char *message, int err_code);

/* Output the server's hostname to standard output */
void displayhostinfo(const char *PORT);

/* Resolve the listener */
void resolve_listener(const char *PORT, struct addrinfo *hints, struct addrinfo **addrs);

/* Establish the server's socket */
void establish_socket(struct addrinfo *addrs, int *listen_fd);

/* Start the server listening on the given fd */
void listen_on_fd(int *listen_fd);

/* Accept a new user and return the file descriptor pointing to their connection */
int accept_new_user(int listen_fd, struct sockaddr_storage *new_address);

/* Add a user with a given fd and sockaddr to the list of users */
void add_user(int fd, struct sockaddr_storage *address, struct user **users);

/* Find the user associated with the given fd */
struct user *get_user(struct user *users, int fd);

/* Handle the message that was received */
void handle_message(struct user **users, struct user *sender, struct cJSON *recvJSON, fd_set *master);

/* Generate text saying that the given user has left the chat room */
char *generate_user_left_message(struct user *sender);

/* Send help text to the given user */
void send_help_text(struct user *user);

/* Initialize the new user's information and send a message that the user has entered the chat room */
void initialize_user(struct user *new_user, char *name, struct user *users);

/* Send a list of currently connected users to the given user */
void send_who_list(struct user *all_users, struct user *requester);

/* Send a message to all users except for the user who initially sent the message */
void send_to_all(struct user *users, char *send_msg, struct user *sender);

/* Send a message to the specified user */
void send_to_user(char *send_msg, struct user *user);

/* Remove a user from a linked list of users */
void remove_user(struct user **users, struct user *user_to_remove);

#endif
