CFLAGS = -Wall -Werror -g
CLIB = -lm -lpthread

all: server client
	rm -f *.o *~

server: server.c server_messaging
	gcc server.c cJSON.c  $(CFLAGS) -o server $(CLIB) -L. -ls_msg -lserver -lkeepalive

server_messaging: server_messaging.c keepalive server_utils
	gcc -c $(CFLAGS) server_messaging.c -L. -lkeepalive -lserver
	ar rcu libs_msg.a server_messaging.o
	rm server_messaging.o

server_utils: server_utils.c keepalive
	gcc -c $(CFLAGS) server_utils.c -L. -lkeepalive
	ar rcu libserver.a server_utils.o
	rm server_utils.o

keepalive: keepalive.c
	gcc -c $(CFLAGS) keepalive.c
	ar rcu libkeepalive.a keepalive.o
	rm keepalive.o

client: client.c client_messaging
	gcc client.c cJSON.c $(CFLAGS) -o client $(CLIB) -L. -lc_msg -lclient -lkeepalive

client_messaging: client_messaging.c client_utils keepalive
	gcc -c $(CFLAGS) client_messaging.c -L. -lkeepalive
	ar rcu libc_msg.a client_messaging.o
	rm client_messaging.o

client_utils: client_utils.c keepalive
	gcc -c $(CFLAGS) client_utils.c -L. -lkeepalive
	ar rcu libclient.a client_utils.o
	rm client_utils.o

clean:
	rm -f *~ server client *.o *.a
