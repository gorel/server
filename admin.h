#ifndef ADMIN_H
#define ADMIN_H

#include "server_messaging.h"

/* This will be called within handle_message -- find out if the user is performing an admin action and act accordingly
 * Return TRUE if the user was performing an admin action, FALSE otherwise
 */
bool handle_admin_action(struct user **users, fd_set *master, struct user *sender, char *msg);

/* Send admin help text to the given user (will check that user is an admin) */
void send_admin_help_text(struct user *user);

/* Promote the specified user to an administrator (will check that admin has the correct privileges) */
void promote_to_admin(struct user *users, struct user *admin, struct user *user_to_promote);

/* Send a message to all users that a new admin has been added to the server */
void send_new_admin_message(struct user *users, char *new_admin_name);

/* Mute the given user (will check that admin has the correct privileges) */
void mute(struct user *admin, struct user *user_to_mute);

/* Send a message to the given user telling them they have been muted by the given admin */
void send_mute_message(struct user *user, struct user *admin);

/* Unmute the given user (will check that admin has the correct privileges) */
void unmute(struct user *admin, struct user *user_to_unmute);

/* Send a message to the given user telling them they have been unmuted by the given admin */
void send_unmute_message(struct user *user, struct user *admin);

/* Kick the given user out of the chat room (will check that admin has the correct privileges) */
void kick(struct user **users, fd_set *master, struct user *admin, struct user *user_to_kick, char *reason);

/* Tell the specified user that they do not have admin privileges */
void send_not_admin_message(struct user *user);

/* Tell the specified admin that they are trying to mute/kick an admin, which is not allowed */
void send_user_is_admin_message(struct user *user);

#endif
