CFLAGS = -Wall -Werror -g
CLIB = -lm -lpthread

all: server client
	rm -f *.o *~

server: server.c handle_message 
	gcc server.c cJSON.c  $(CFLAGS) -o server $(CLIB) -L. -lserver 

handle_message: handle_message.c admin server_messaging
	gcc -c $(CFLAGS) handle_message.c 
	ar rcu libserver.a handle_message.o admin.o server_messaging.o server_utils.o keepalive.o

admin: admin.c server_messaging
	gcc -c $(CFLAGS) admin.c 

server_messaging: server_messaging.c keepalive server_utils
	gcc -c $(CFLAGS) server_messaging.c 

server_utils: server_utils.c keepalive
	gcc -c $(CFLAGS) server_utils.c 

keepalive: keepalive.c
	gcc -c $(CFLAGS) keepalive.c


client: client.c client_messaging
	gcc client.c cJSON.c $(CFLAGS) -o client $(CLIB) -lpthread -lrt -L. -lclient

client_messaging: client_messaging.c client_utils keepalive
	gcc -c $(CFLAGS) client_messaging.c 
	ar rcu libclient.a client_messaging.o client_utils.o keepalive.o

client_utils: client_utils.c keepalive
	gcc -c $(CFLAGS) client_utils.c 

clean:
	rm -f *~ server client *.o *.a
