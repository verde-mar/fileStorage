#!/bin/bash

# Creo le directory in cui memorizzare i file letti e quelli eliminati dal server
mkdir ./flushed ./read

# Avvia il server
./smain ./files/file_config.txt ./files/log_file.txt &

sleep 3

# Salva il pid del server
pid=$!

# Avvia i client
./cl -f socket -p -t 100 -D ./flushed -W ./test_directory/prova.txt -D ./flushed -W ./test_directory/prova.txt,./test_directory/prova2.txt -D ./flushed -W ./test_directory/prova2.txt &
pidcl1=$!
./cl -f socket -p -t 125 -D ./flushed -W ./test_directory/prova8.txt,./test_directory/prova3.txt -D ./flushed -W ./test_directory/prova6.txt -D ./flushed -W ./test_directory/prova.txt &
pidcl2=$!
./cl -f socket -p -t 100 -D ./flushed -W ./test_directory/prova11.txt,./test_directory/prova11.txt,./test_directory/prova11.txt &
pidcl3=$!
./cl -f socket -p -t 125 -D ./flushed -W ./test_directory/prova2.txt -D ./flushed -W ./test_directory/prova9.txt -D ./flushed -W ./test_directory/prova5.txt &
pidcl4=$!

wait $pidcl1 $pidcl2 $pidcl3 $pidcl4

# Terminazione lenta del server
kill -1 $pid

# Aspetta il server
wait $pid

exit 0