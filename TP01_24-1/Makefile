all: 
	mkdir -p bin
	gcc -Wall -c common.c
	gcc -Wall client.c common.o -o bin/client
	gcc -Wall server.c -lm common.o -o bin/server