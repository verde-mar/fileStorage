#!/bin/bash

# Avvia il server
./smain ./file_config.txt ./log_file.txt &

sleep 3

# Salva il pid del server
pid=$!

for (( i=1; i<=5; i++ )); do
    ./cl -f socket -t 0 -D ./flushed -w ./test_directory/prova.txt -d ./read -R 1 -D ./flushed -W ./test_directory/ -l ./test_directory/prova10.txt -u ./test_directory/prova10.txt &
    ./cl -f socket -t 0 -d ./read -r ./test_directory/prova4.txt -D ./flushed -W ./test_directory/ -l ./test_directory/prova2.txt -u ./test_directory/prova2.txt -c ./test_directory/prova8.txt &
    ./cl -f socket -t 0 -l ./test_directory/prova6.txt -u ./test_directory/prova6.txt -d ./read -R 0 &

done

sleep 30

# Terminazione lenta del server
kill -2 $pid

# Aspetta il server
wait $pid

exit 0