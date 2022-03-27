#!/bin/bash

# Creo le directory in cui memorizzare i file letti e quelli eliminati dal server
mkdir ./flushed ./read

# Avvia il server
valgrind -s ./smain ./files/file_config.txt ./files/log_file.txt &

sleep 2

# Salva il pid del server
pid=$!

# Avvia i client
./cl -f socket -p -t 100 -D ./flushed -W ./test_dir1/prova.txt,./test_dir1/prova.txt  &
pidcl1=$!
./cl -f socket -p -t 125 -D ./flushed -W ./test_dir1/prova3.txt -D ./flushed -W ./test_dir1/prova6.txt -D ./flushed -W ./test_dir2/sea2.jpg &
pidcl2=$!
./cl -f socket -p -t 100 -D ./flushed -W ./test_dir1/prova11.txt,./test_dir1/prova11.txt,./test_dir1/prova11.txt,./test_dir1/prova11.txt &
pidcl3=$!
./cl -f socket -p -t 125 -D ./flushed -W ./test_dir1/prova2.txt -D ./flushed -W ./test_dir2/sea1.jpg &
pidcl4=$!

wait $pidcl1 $pidcl2 $pidcl3 $pidcl4

# Terminazione lenta del server
kill -1 $pid

# Aspetta il server
wait $pid

exit 0