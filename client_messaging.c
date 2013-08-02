#include "client_messaging.h"
	
/* Receive the initial message from the server and output it to console */
bool receive_initial_message(int server_fd)
{
    int num_bytes;
    char msg[MAXLEN];
    
	//Receive the server's initial message
	if ((num_bytes = recv(server_fd, msg, MAXLEN - 1, 0)) == -1)
	{
		perror("Receive");
		exit(ERROR_WITH_RECV);
	}
    
    //Ensure the JSON string is null terminated
    msg[num_bytes] = '\0';
    
    //If any bytes were received, parse the message and output the information
    if (num_bytes > 0)
    {
    	//Use cJSON to parse the message we received
        cJSON  *recvJSON = cJSON_Parse(msg);
        
        //Extract the JSON data
        int mlen = cJSON_GetObjectItem(recvJSON, "mlen")->valueint;
        char *msg = cJSON_GetObjectItem(recvJSON, "msg")->valuestring;
        bool valid = cJSON_GetObjectItem(recvJSON, "valid")->valueint;
        
        //Ensure the message is null terminated
        msg[mlen] = '\0';
        
        //Print out the message being sent
        printf("%s\n", msg);
                
        cJSON_Delete(recvJSON);
        
        //Return whether or not the username was valid
        return valid;
    }
    //If no bytes were received in the initial message, there is a connection error
    else
    {
    	fprintf(stderr, "Connection error: No response from server.\n");
    	exit(EMPTY_RESPONSE_FROM_SERVER);
    	return EMPTY_RESPONSE_FROM_SERVER;
    }
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
    sem_t *exit_signal = args->exit_signal;
    
    //Loop forever to receive messages
    while (1)
    {
    	int sem_val;
    	
    	//Get the current semaphore value and store it in sem_val
		sem_getvalue(exit_signal, &sem_val);
	
		//If the semaphore has been decremented to 0, exit the while loop (main thread sent exit signal)
		if (sem_val == 0)
			break;
    
    	//Receive the next message
    	if ((num_bytes = recv(server_fd, msg, MAXLEN - 1, 0)) == -1)
    	{
    		perror("Receive");
    		exit(ERROR_WITH_RECV);
    	}
        
        //Ensure the JSON string is null terminated
        msg[num_bytes] = '\0';
        
        //If any bytes were received, parse the message and output the information
        if (num_bytes > 0)
        {        
        	//Use cJSON to parse the message we received
            cJSON  *recvJSON = cJSON_Parse(msg);
            bool kicked = FALSE;
            
            //Extract the JSON data
            char *from = cJSON_GetObjectItem(recvJSON, "from")->valuestring;
            int mlen = cJSON_GetObjectItem(recvJSON, "mlen")->valueint;
            char *msg = cJSON_GetObjectItem(recvJSON, "msg")->valuestring;
            bool private = cJSON_GetObjectItem(recvJSON, "private")->valueint;
            
            //Only checked for the "kicked" signal if the sender was the server (Why would you trust any random client to tell you that you've been kicked?)
            if (!strcmp(from, "SERVER"))
            	kicked = cJSON_GetObjectItem(recvJSON, "kicked")->valueint;
            
            //Ensure that the msg is null terminated
            msg[mlen] = '\0';
            
            //If the user was kicked from the server, decrement the semaphore and break out of the while loop 
            if (kicked)
            {
				//Free the recvJSON object
				cJSON_Delete(recvJSON);
			
				//Decrement the semaphore to signal the receiver thread to end then break out of the while loop
				sem_wait(exit_signal);
				break;
            }
            
            //Print out the message being sent
            if (!strcmp(from, "SERVER"))
            	printf("%s\n", msg);
            else if (private)
            	printf("%s whispers: %s\n", from, msg);
           	else
           		printf("%s: %s\n", from, msg);
                    
            //Free the JSON object
            cJSON_Delete(recvJSON);
        }
    }    
    
    //Exit with normal status
    pthread_exit(0);
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
	{
		perror("Send");
		exit(ERROR_WITH_SEND);
	}
		
	//Delete the cJSON Object and free send_msg
	cJSON_Delete(sendJSON);
	free(send_msg);
}

/* Wait for the user to input text then send it to the server */
int send_new_message(int server_fd, char *name)
{
	int num_bytes;
	char msg[MAXMSG];
	
	//Create the cJSON object which will be sent
	cJSON *sendJSON = cJSON_CreateObject();
	
	//Read a message in from the user
	if ((num_bytes = read(STDIN_FILENO, msg, MAXMSG - 1)) < 0)
	{
		perror("Read");
		exit(ERROR_WITH_READ);
	}
	
	//Don't allow a message that is just a newline to be sent (return 0 because this isn't an error, though)
	if (msg[0] == '\n')
		return 0;
	
	//Null-terminate the message (and remove the trailing newline character)
	msg[num_bytes - 1] = '\0';
	
	//Add the message and message length to the JSON object
	cJSON_AddStringToObject(sendJSON, "from", name);
	cJSON_AddNumberToObject(sendJSON, "mlen", num_bytes - 1);
	cJSON_AddStringToObject(sendJSON, "msg", (char *)msg);
	
	//Print out the JSON object to string
	char *send_msg = cJSON_Print(sendJSON);	
	
	//Send the message to the server
	if ((send(server_fd, send_msg, strlen(send_msg), 0)) == -1)
	{
		perror("Send");
		exit(ERROR_WITH_SEND);
	}
		
	//Delete the cJSON Object and free send_msg
	cJSON_Delete(sendJSON);
	free(send_msg);
	
	//If the user chose to quit, return QUIT_OPTION
	if (!strcmp("!quit", msg))
		return QUIT_OPTION;

	//Otherwise, return 0
	return 0;
}
