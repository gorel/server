#include "keepalive.h"

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
