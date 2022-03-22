#!/bin/bash

# Avvia il server
./smain ./file_config.txt ./log_file.txt &

sleep 3

# Salva il pid del server
pid=$!

# Avvia i client
./cl -f socket -p -t 100 -D ./flushed -w ./test_directory/prova7.txt -D ./flushed -w ./test_directory/prova.txt  -D ./flushed -w ./test_directory/prova2.txt -D ./flushed -w ./test_directory/prova2.txt &
pidcl1=$!
./cl -f socket -p -t 125 -D ./flushed -w ./test_directory/prova8.txt -D ./flushed -w ./test_directory/prova3.txt -D ./flushed -w ./test_directory/prova5.txt  -D ./flushed -w ./test_directory/prova6.txt -D ./flushed -w ./test_directory/prova.txt -D ./flushed -w ./test_directory/prova3.txt &
pidcl2=$!
./cl -f socket -p -t 100 -D ./flushed -W ./test_directory -D ./flushed -w ./test_directory/prova4.txt -D ./flushed -w ./test_directory/prova5.txt &
pidcl3=$!
./cl -f socket -p -t 1250 -D ./flushed -w ./test_directory/prova3.txt -D ./flushed -w ./test_directory/prova2.txt -D ./flushed -w ./test_directory/prova9.txt -D ./flushed -w ./test_directory/prova5.txt &
pidcl4=$!

wait $pidcl1 $pidcl2 $pidcl3 $pidcl4

# Terminazione lenta del server
kill -2 $pid

# Aspetta il server
wait $pid

exit 0