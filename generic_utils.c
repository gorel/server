#include "generic_utils.h"

/* Set up TCP Keepalive */
void keepalive(int *listen_fd, int output_on)
{
	int n, r, optval;
	socklen_t optlen = sizeof(optval);
	
	//Set up SO_KEEPALIVE
	optval = 1;
	r = setsockopt(*listen_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
	assert (r != -1);
	r = getsockopt(*listen_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, &optlen);
	assert (r != -1);
	
	if (output_on)
		printf("<K> SO_KEEPALIVE is %s\n", (optval ? "ON" : "OFF" ));
	
	//Set up TCP_KEEPCNT
	n = NUM_TCP_KEEPCNT;
	r = setsockopt(*listen_fd, SOL_TCP, TCP_KEEPCNT, (char *)&n, sizeof(n));
	assert (r != -1);
	r = getsockopt(*listen_fd, SOL_TCP, TCP_KEEPCNT, &optval, &optlen);
	assert (r != -1);
	
	if (output_on)
		printf("<K> TCP_KEEPCNT is %s\n", (optval ? "ON" : "OFF" ));
	
	//Set up TCP_KEEPIDLE
	n = NUM_TCP_KEEPIDLE;
	r = setsockopt(*listen_fd, SOL_TCP, TCP_KEEPIDLE, (char *)&n, sizeof(n));
	assert (r != -1);
	r = getsockopt(*listen_fd, SOL_TCP, TCP_KEEPIDLE, &optval, &optlen);
	assert (r != -1);
	
	if (output_on)
		printf("<K> TCP_KEEPIDLE is %s\n", (optval ? "ON" : "OFF" ));
	
	//Set up TCP_KEEPINTVL
	n = NUM_TCP_KEEPINTVL;
	r = setsockopt(*listen_fd, SOL_TCP, TCP_KEEPINTVL, (char *)&n, sizeof(n));
	assert (r != -1);
	r = getsockopt(*listen_fd, SOL_TCP, TCP_KEEPINTVL, &optval, &optlen);
	assert (r != -1);
	
	if (output_on)
		printf("<K> TCP_KEEPIDLE is %s\n", (optval ? "ON" : "OFF" ));
}

/* Allocate a JSON string with the given data */
char *allocate_json_string(char *from, char *msg, bool valid, bool private, bool kicked)
{
	//Create a cJSON object to organize the data
	cJSON *json = cJSON_CreateObject();
    
    //Fill in the JSON data
    cJSON_AddStringToObject(json, "from", from);
    cJSON_AddNumberToObject(json, "mlen", strlen(msg));
    cJSON_AddStringToObject(json, "msg", msg);
    cJSON_AddNumberToObject(json, "valid", valid);
    cJSON_AddNumberToObject(json, "private", private);
    cJSON_AddNumberToObject(json, "kicked", kicked);
    
    //Get the JSON data in string format
    char *json_string = cJSON_Print(json);
    
    //Delete the cJSON object and return msg
    cJSON_Delete(json);
    return json_string;
}
