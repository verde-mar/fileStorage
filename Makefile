CC = gcc -std=c99 -O3 -g 
CFLAGS = -Wall -pthread -D_POSIX_C_SOURCE=200112L

CLIENT = ./client
SERVER = ./server
UTILS = ./utils

.PHONY: clean test1 test2 test3

all: smain cl

$(SERVER)/libserver.so: $(SERVER)/fifo.o $(SERVER)/queue.o $(SERVER)/hash.o $(SERVER)/threadpool.o $(SERVER)/gestore.o $(UTILS)/utils.o  $(UTILS)/socketIO.o
	$(CC) $(CFLAGS) -fPIC -I $(SERVER) -I $(UTILS) -shared -o $@ $^

smain: $(SERVER)/server_main.c $(SERVER)/libserver.so
	$(CC) $(CFLAGS) -fPIC -I $(SERVER) -I $(UTILS) -o $@ $^

$(CLIENT)/libclient.so: $(CLIENT)/worker.o $(UTILS)/utils.o $(UTILS)/socketIO.o $(UTILS)/client_utils.o
	$(CC) $(CFLAGS) -fPIC -I $(CLIENT) -I $(UTILS) -shared -o $@ $^

$(CLIENT)/libcaller.so: $(CLIENT)/dispatcher.o
	$(CC) $(CFLAGS) -fPIC -I $(CLIENT) -I $(UTILS) -shared -o $@ $^

cl: $(CLIENT)/main.c $(CLIENT)/libcaller.so $(CLIENT)/libclient.so
	$(CC) $(CFLAGS) -fPIC -I $(CLIENT) -I $(UTILS) -o $@ $^

%.o: %.c %.h
	$(CC) $(CFLAGS) -fPIC -I $(SERVER) -I $(CLIENT) -I $(UTILS) -c -o $@ $<

clean:
	rm ./server/*.o ./server/*.so smain ./utils/*.o socket ./client/*.o ./client/*.so cl ./read/* ./flushed/*

test1: all
	./script/create_config.sh socket 10000 128000000 1 ./file_config.txt
	./script/primo_test.sh

test2: all
	./script/create_config.sh socket 10 1000000 4 ./file_config.txt
	./script/secondo_test.sh

test3: all
	./script/create_config.sh socket 100 32000000 8 ./file_config.txt
	./script/terzo_test.sh
	