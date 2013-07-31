#ifndef CLIENT_MESSAGING_H
#define CLIENT_MESSAGING_H

#include "client_utils.h"

/* Receive the initial message from the server and output it to console */
bool receive_initial_message(int server_fd);

/* Create an extra thread to output when a new message is received
 * This allows the user to send messages and receive them in real time
 * instead of having an awkward "all-at-once" kind of connection
 */
void *receive(void *thread_data);

/* Send initial data to the server */
void send_initial_message(int server_fd, char *name);

/* Wait for the user to input text then send it to the server */
int send_new_message(int server_fd, char *name);

#endif
