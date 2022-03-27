#!/bin/bash

# Creo le directory in cui memorizzare i file letti e quelli eliminati dal server
mkdir ./flushed ./read

# Avvia il server
valgrind --leak-check=full ./smain ./files/file_config.txt ./files/log_file.txt &

sleep 2

# Salva il pid del server
pid=$!

# Avvia i client
./cl -f socket -p -t 200 -D ./flushed -W ./test_dir1/prova.txt,./test_dir1/prova2.txt,./test_dir1/prova4.txt -d ./read -r ./test_dir1/prova.txt -l ./test_dir1/prova3.txt -u ./test_dir1/prova3.txt -R 0 -c ./test_dir1/prova2.txt &
pidcl1=$!
./cl -f socket -p -t 200 -D ./flushed -W ./test_dir1/prova3.txt -R 0 -d ./read -r ./test_dir1/prova5.txt,./test_dir1/sea.jpg -D ./flushed -w ./test_dir2/ -l ./test_dir1/prova2.txt,./test_dir1/prova4.txt -u ./test_dir1/prova2.txt,./test_dir1/prova4.txt -R 2 &
pidcl2=$!

wait $pidcl1 $pidcl2

# Terminazione lenta del server
kill -1 $pid

# Aspetta il server
wait $pid

exit 0