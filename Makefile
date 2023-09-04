CC=gcc
CFLAGS=-I.

all: client

parson.o: parson.c

client: client.c requests.c helpers.c buffer.c parson.o
	$(CC) -o client client.c requests.c helpers.c buffer.c parson.o -Wall

run: client
	./client

clean:
	rm -f *.o client
