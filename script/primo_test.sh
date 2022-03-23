#!/bin/bash

# Avvia il server
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./smain ./file_config.txt ./log_file.txt &

sleep 3

# Salva il pid del server
pid=$!

# Avvia i client
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./cl -f socket -p -t 200 -D ./flushed -w ./test_directory/prova.txt -d ./read -r ./test_directory/prova.txt -D ./flushed -W ./test_directory/ -l ./test_directory/prova2.txt -u ./test_directory/prova2.txt -c ./test_directory/prova2.txt &
#./cl -f socket -p -t 200 -D ./flushed -w ./test_directory/prova.txt -l ./test_directory/prova.txt -u ./test_directory/prova.txt &
pidcl1=$!
valgrind --track-origins=yes --leak-check=full --show-leak-kinds=all ./cl -f socket -p -t 200 -D ./flushed -w ./test_directory/prova3.txt -d ./read -r ./test_directory/prova.txt -D ./flushed -W ./test_directory/ -l ./test_directory/prova2.txt -u ./test_directory/prova2.txt -c ./test_directory/prova.txt &
#./cl -f socket -p -t 200 -w ./test_directory/prova2.txt -l ./test_directory/prova.txt -u ./test_directory/prova.txt &
pidcl2=$!

wait $pidcl1 $pidcl2

# Terminazione lenta del server
kill -2 $pid

# Aspetta il server
wait $pid

exit 0