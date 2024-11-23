CC = gcc
CFLAGS = -Wall -std=c99 -pedantic
CLIENT = client
CLIENT_OBJS = client.o
SERVER = httpd
SERVER_OBJS = httpd.o net.o cgi_like.o
PROGS = $(CLIENT) $(SERVER)

all : $(PROGS)

$(CLIENT) : $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_OBJS)

client.o : client.c
	$(CC) $(CFLAGS) -c client.c

$(SERVER) : $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_OBJS)

server.o : httpd.c net.h cgi_like.h
	$(CC) $(CFLAGS) -c httpd.c

net.o : net.c net.h
	$(CC) $(CFLAGS) -c net.c

cgi_like.o : cgi_like.c cgi_like.h
	$(CC) $(CFLAGS) -c cgi_like.c


clean :
	rm *.o $(PROGS) core
