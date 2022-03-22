#!/bin/bash

valgrind --track-origins=yes --main-stacksize=1000000000 ./smain ./file_config.txt ./log_file.txt &

sleep 3

# mi salvo il pid ($! viene sostituito col PID del processo più recente avviato in background)
pid=$!


#chiama i client
./cl -f socket -p -t 200 -D ./flushed -w ./test_directory/prova.txt -d ./read -r ./test_directory/prova.txt -D ./flushed -w ./test_directory/prova2.txt -l ./test_directory/prova2.txt -u ./test_directory/prova2.txt &
pidcl1=$!
./cl -f socket -p -t 200 -c ./test_directory/prova3.txt -r ./test_directory/prova.txt &
pidcl2=$!
./cl -f socket -p -t 200 -d ./read -R 0 -D ./flushed -w ./test_directory/sea.jpg  &
pidcl3=$!
#./cl -f socket -p -t 200 -d ./test_directory -D ./flushed -w ./test_directory/sea.jpg -W ./test_directory -R 3 -c ./test_directory/sea.jpg -W ./test_directory -r ./test_directory/prova.txt &
#pidcl4=$!
#./cl -f socket -p -t 200 -d ./test_directory -D ./flushed -w ./test_directory/sea.jpg -W ./test_directory -R 3 -c ./test_directory/sea.jpg -W ./test_directory -r ./test_directory/prova2.txt &

wait $pidcl1 $pidcl2 $pidcl3

# terminazione lenta al server
kill -2 $pid

# aspetto il server
wait $pid

exit 0