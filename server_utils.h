#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include "keepalive.h"
#include "cJSON.h"
#include "return_vals.h"

#define MAX_WAIT 10
#define MAXLEN 1024
#define MAXMSG 512
#define MAXIGNORE 20

#define EARTH_IS_THIRD_PLANET_FROM_THE_SUN 1

#define TRUE 1
#define FALSE 0

typedef char bool;

struct user
{
	int fd;
	char *name;
	bool afk;
	bool admin;
	bool muted;
	
	struct user *ignore_list[MAXIGNORE];
	
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
struct user *get_user_by_fd(struct user *users, int fd);

/* Find the user associated with the given name */
struct user *get_user_by_name(struct user *users, char *name);

/* Find out if a given username is already in use in the linked list of users */
bool username_already_in_use(struct user *users, char *name);

/* Remove a user from a linked list of users */
void remove_user(struct user **users, struct user *user_to_remove);

/* Update the user's ignore list to include the new user */
void ignore(struct user *user, struct user *user_to_ignore);

/* Update the user's ignore list to remove the given user */
void unignore(struct user *user, struct user *user_to_unignore);

/* Return whether or not the given user is ignoring <target> */
bool ignoring(struct user *user, struct user *target);

#endif
