CC = gcc
CFLAGS = -Wall -std=c99 -pedantic
SERVER = httpd
SERVER_OBJS = httpd.o net.o cgi_like.o
PROGS = $(SERVER)

all : $(SERVER)

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

test :
	$(CC) $(CFLAGS) test.c -o test

clean :
	rm *.o $(PROGS) core
