CC = gcc
CFLAGS =  -Wall  -lssl -lcrypto -lpthread
INC = -I /usr/foo/include
LIB = -L/usr/lib   

all: client server
client:
	${CC} threadClient.c ${CFLAGS}  ${LIB}  -o threadClient
server:
	gcc threadServer.c -o threadServer -l pthread -std=c99
run:
	./threadServer
run1:
	./threadClient 127.0.0.1 8889
clean:
	rm -rf threadClient
	rm -rf threadServer
