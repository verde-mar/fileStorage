#!/bin/bash

# Creo le directory in cui memorizzare i file letti e quelli eliminati dal server
mkdir ./flushed ./read

# Avvia il server
./smain ./files/file_config.txt ./files/log_file.txt &

sleep 3

# Salva il pid del server
pid=$!

for (( i=1; i<=15; i++ )); do
    ./cl -f socket -t 0 -D ./flushed -w ./test_dir2/ -D ./flushed -W ./test_dir1/prova.txt -D ./flushed -w ./test_dir1/ -l ./test_dir1/prova10.txt -u ./test_dir1/prova10.txt &
    ./cl -f socket -t 0 -d ./read -r ./test_dir1/prova4.txt -D ./flushed -l ./test_dir1/prova2.txt -u ./test_dir1/prova2.txt -c ./test_dir1/prova.txt -W ./test_dir2/prova12.txt &
    ./cl -f socket -t 0 -D ./flushed -W ./test_dir1/prova3.txt -l ./test_dir1/prova6.txt -u ./test_dir1/prova6.txt -d ./read -R 0 &
done

sleep 30

# Terminazione lenta del server
kill -2 $pid

# Aspetta il server
wait $pid

exit 0