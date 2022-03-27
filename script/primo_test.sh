#!/bin/bash

# Creo le directory in cui memorizzare i file letti e quelli eliminati dal server
mkdir ./flushed ./read

# Avvia il server
valgrind --leak-check=full ./smain ./files/file_config.txt ./files/log_file.txt &

sleep 3

# Salva il pid del server
pid=$!

# Avvia i client
./cl -f socket -p -t 200 -D ./flushed -W ./test_directory/prova.txt,./test_directory/prova2.txt,./test_directory/prova4.txt -d ./read -r ./test_directory/prova.txt -D ./flushed -w ./test_directory/ -l ./test_directory/prova2.txt -u ./test_directory/prova2.txt -R 0 -c ./test_directory/prova2.txt,./test_directory/prova8.txt &
pidcl1=$!
./cl -f socket -p -t 200 -D ./flushed -W ./test_directory/prova3.txt -d ./read -r ./test_directory/prova.txt,./test_directory/prova2.txt -D ./flushed -w ./test_directory/ -l ./test_directory/prova2.txt,./test_directory/prova4.txt -u ./test_directory/prova2.txt,./test_directory/prova4.txt -c ./test_directory/prova2.txt -R 2 &
pidcl2=$!

wait $pidcl1 $pidcl2

# Terminazione lenta del server
kill -1 $pid

# Aspetta il server
wait $pid

exit 0