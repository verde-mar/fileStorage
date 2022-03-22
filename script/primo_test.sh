#!/bin/bash

# Avvia il server
valgrind --track-origins=yes ./smain ./file_config.txt ./log_file.txt &

sleep 3

# Salva il pid del server
pid=$!

# Avvia i client
./cl -f socket -p -t 200 -D ./flushed -w ./test_directory/prova.txt -d ./read -r ./test_directory/prova.txt -D ./flushed -W ./test_directory/ -l ./test_directory/prova2.txt -u ./test_directory/prova2.txt -d ./read -R 0 -c ./test_directory/prova2.txt &
pidcl1=$!
./cl -f socket -p -t 200 -D ./flushed -w ./test_directory/prova3.txt -d ./read -r ./test_directory/prova.txt -D ./flushed -W ./test_directory/ -l ./test_directory/prova2.txt -u ./test_directory/prova2.txt -d ./read -R 0 -c ./test_directory/prova.txt &
pidcl2=$!

wait $pidcl1 $pidcl2

# Terminazione lenta del server
kill -2 $pid

# Aspetta il server
wait $pid

exit 0