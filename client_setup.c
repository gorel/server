#include "client_setup.h"
#include <stdio.h>

/* Print out the given error message and exit the program with the given error code */
void error(char *message, int err_code)
{
	fprintf(stderr, "%s\n", message);
	exit(err_code);
}

/* Get the user's name */
char *get_name(void)
{
	int num_bytes;
	
	//Allocate space for the user's name
	char *name = (char *)malloc(sizeof(char) * NAMELEN);
	
	//Prompt the user for their name -- use stderr for unbuffered output
	fprintf(stderr, "Name: ");
	num_bytes = read(STDIN_FILENO, name, NAMELEN);
	
	//Null-terminate the name (and strip off the trailing newline character)
	name[num_bytes - 1] = '\0';
	
	return name;
}

/* Resolve the listener */
void get_addr_info(const char *HOSTNAME, const char *PORT, struct addrinfo *hints, struct addrinfo **addrs)
{
	int err;
	if ((err = getaddrinfo(HOSTNAME, PORT, hints, addrs)) != 0)
	{
		perror("Error getting address info");
		exit(2);
	}
}

/* Establish the socket and connect to the host */
void establish_connection(int *new_fd, struct addrinfo *addrs)
{
	//Establish the socket
	if ((*new_fd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol)) == -1)
    {
            perror("Error establishing socket");
            exit(3);
    }
    
    //Connect to the host
    if (connect(*new_fd, addrs->ai_addr, addrs->ai_addrlen) == -1)
    {
            perror("Error connecting to host");
            exit(4);
    }
    
    //Set up TCP Keepalive
    keepalive(new_fd);
}

/* Create an extra thread to output when a new message is received
 * This allows the user to send messages and receive them in real time
 * instead of having an awkward "all-at-once" kind of connection
 */
void *receive(void *thread_data)
{       
    struct thread_data *args = (struct thread_data *)thread_data;
    
    int num_bytes;
    char msg[MAXLEN];
    
    //Parse the data from the arguments
    int server_fd = args->fd;
    
    //Loop forever to receive messages
    while (1)
    {
    	//Receive the next message
    	if ((num_bytes = recv(server_fd, msg, MAXLEN - 1, 0)) == -1)
    		perror("Receive");
        
        //Ensure the JSON string is null terminated
        msg[num_bytes] = '\0';
        
        //If any bytes were received, parse the message and output the information
        if (num_bytes > 0)
        {
        	//Use cJSON to parse the message we received
            cJSON  *result = cJSON_Parse(msg);
            
            //Find the sender of the data received
            char *from = cJSON_GetObjectItem(result, "from")->valuestring;
            //Find the message length of the data received
            int mlen = cJSON_GetObjectItem(result, "mlen")->valueint;
            //Find the actual message of the data received
            char *plaintext = cJSON_GetObjectItem(result, "msg")->valuestring;
            //Ensure the message is null terminated
            plaintext[mlen] = '\0';
            
            //Display the message on standard output
            printf("%s: %s\n", from, plaintext);
                    
            cJSON_Delete(result);
        }
    }
    //Avoid compiler warnings by placing a return NULL statement at the end    
    return NULL;
}

/* Send initial data to the server */
void send_initial_message(int server_fd, char *name)
{
	//Create the cJSON object which will be sent
	cJSON *sendJSON = cJSON_CreateObject();
	
	//Add the user's name to the JSON object
	cJSON_AddStringToObject(sendJSON, "from", name);
	
	//Print out the JSON object to string
	char *send_msg = cJSON_Print(sendJSON);
	
	//Send the message to the server
	if ((send(server_fd, send_msg, strlen(send_msg), 0)) == -1)
		perror("Send");
		
	//Delete the cJSON Object
	cJSON_Delete(sendJSON);
}

/* Wait for the user to input text then send it to the server */
int send_new_message(int server_fd)
{
	int num_bytes;
	char msg[MAXMSG];
	
	//Create the cJSON object which will be sent
	cJSON *sendJSON = cJSON_CreateObject();
	
	//Read a message in from the user
	if ((num_bytes = read(STDIN_FILENO, msg, MAXMSG - 1)) < 0)
		perror("Read");
	
	//Null-terminate the message (and remove the trailing newline character)
	msg[num_bytes - 1] = '\0';
	
	//Add the message and message length to the JSON object
	cJSON_AddNumberToObject(sendJSON, "mlen", num_bytes - 1);
	cJSON_AddStringToObject(sendJSON, "msg", (char *)msg);
	
	//Print out the JSON object to string
	char *send_msg = cJSON_Print(sendJSON);	
	
	//Send the message to the server
	if ((send(server_fd, send_msg, strlen(send_msg), 0)) == -1)
		perror("Send");
		
	//Delete the cJSON Object
	cJSON_Delete(sendJSON);
	
	//If the user chose to quit, return QUIT_OPTION
	if (!strcmp("!quit", msg))
		return QUIT_OPTION;

	//Otherwise, return 0
	return 0;
}
