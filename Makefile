.PHONY: all client serveur

CC = gcc
EXEC = serveur client

all: client serveur

client:
	$(CC) -g -o $@ $@.c

serveur:
	$(CC) -g -o $@ $@.c
		
clean:
	rm -rf $(EXEC)
