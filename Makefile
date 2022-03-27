CC = gcc -std=c99 -O3 -g 
CFLAGS = -Wall -pthread -D_POSIX_C_SOURCE=200112L 

CLIENT = ./client
SERVER = ./server
UTILS = ./utils

.PHONY: clean test1 test2 test3 

all: smain cl

$(SERVER)/libserver.so: $(SERVER)/fifo_cache.o $(SERVER)/fifo_wait.o $(SERVER)/fifo_req.o  $(SERVER)/queue.o $(SERVER)/hash.o $(SERVER)/threadpool.o $(SERVER)/gestore_segnali.o $(UTILS)/utils.o  $(UTILS)/socketIO.o
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
	./script/cleaner.sh
	
test1: all
	./script/create_config.sh socket 10000 128000000 1 ./files/file_config.txt
	./script/primo_test.sh

test2: all
	./script/create_config.sh socket 10 1000000 4 ./files/file_config.txt
	./script/secondo_test.sh

test3: all
	./script/create_config.sh socket 100 32000000 8 ./files/file_config.txt
	./script/terzo_test.sh
	