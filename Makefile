CC = gcc -std=c99 -O3 -g 
CFLAGS = -Wall -pedantic -pthread

CLIENT = ./client
SERVER = ./server
UTILS = ./utils

#all: cl
all: smain

$(SERVER)/libserver.so: $(SERVER)/fifo.o $(SERVER)/queue.o $(SERVER)/hash.o $(SERVER)/threadpool.o $(SERVER)/gestore.o
	$(CC) $(CFLAGS) -fPIC -I $(SERVER) -I $(UTILS) -shared -o $@ $^

smain: $(SERVER)/server_main.c $(SERVER)/libserver.so
	$(CC) $(CFLAGS) -fPIC -I $(SERVER) -I $(UTILS) -o $@ $^

#$(CLIENT)/libclient.so: $(CLIENT)/dispatcher.o $(CLIENT)/worker.o $(UTILS)/utils.o $(CLIENT)/socketIO.o
#	$(CC) $(CFLAGS) -fPIC -I $(CLIENT) -I $(UTILS) -shared -o $@ $^

#cl: $(CLIENT)/main.c $(CLIENT)/libclient.so
#	$(CC) $(CFLAGS) -fPIC -I $(CLIENT) -I $(UTILS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -fPIC -I $(SERVER) -I $(UTILS) -c -o $@ $<