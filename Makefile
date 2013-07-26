CFLAGS = -Wall -Werror -g
CLIB = -lm -lpthread

all: server client
	rm -f *.o *~

server: server.c server_setup
	gcc server.c cJSON.c  $(CFLAGS) -o server $(CLIB) -L. -lserver -lkeepalive

server_setup: server_setup.c keepalive
	gcc -c $(CFLAGS) server_setup.c -L. -lkeepalive
	ar rcu libserver.a server_setup.o
	rm server_setup.o

keepalive: keepalive.c
	gcc -c $(CFLAGS) keepalive.c
	ar rcu libkeepalive.a keepalive.o
	rm keepalive.o

client: client.c client_setup
	gcc client.c cJSON.c $(CFLAGS) -o client $(CLIB) -L. -lclient -lkeepalive

client_setup: client_setup.c keepalive
	gcc -c $(CFLAGS) client_setup.c -L. -lkeepalive
	ar rcu libclient.a client_setup.o
	rm client_setup.o

clean:
	rm -f *~ server client *.o *.a
