.PHONY: all client server

CC = gcc
EXEC = server client

all: client server

client:
	$(CC) -g -o $@ $@.c

server:
	$(CC) -g -o $@ $@.c
		
clean:
	rm -rf $(EXEC)
