CC=gcc
CFLAGS=-I. -pthread -Wall -Werror
DEPS = utils.h
OBJ_CLIENT = client.o utils.o
OBJ_SERVER = server.o utils.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

client: $(OBJ_CLIENT)
	$(CC) -o $@ $^ $(CFLAGS)

server: $(OBJ_SERVER)
	$(CC) -o $@ $^ $(CFLAGS)

all: client server

clean:
	rm -f *.o client server

.PHONY: all clean
