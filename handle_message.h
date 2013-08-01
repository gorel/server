#ifndef HANDLE_MESSAGE_H
#define HANDLE_MESSAGE_H

#include "server_messaging.h"
#include "admin.h"

/* Handle the message that was received */
void handle_message(struct user **users, struct user *sender, struct cJSON *recvJSON, fd_set *master);

#endif
