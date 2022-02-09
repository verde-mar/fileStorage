CC = gcc -std=c99 -O3 -g 
CFLAGS = -Wall -pedantic -pthread

CLIENT = ./client
SERVER = ./server
UTILS = ./utils

all: cl

# Librerie client
$(CLIENT)/libclient.so: $(CLIENT)/dispatcher.o $(CLIENT)/worker.o $(UTILS)/utils.o $(CLIENT)/socketIO.o
	$(CC) $(CFLAGS) -fPIC -I $(CLIENT) -I $(UTILS) -shared -o $@ $^

# Eseguibile client
cl: $(CLIENT)/main.c $(CLIENT)/libclient.so
	$(CC) $(CFLAGS) -fPIC -I $(CLIENT) -I $(UTILS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -fPIC -I $(CLIENT) -I $(UTILS) -c -o $@ $<