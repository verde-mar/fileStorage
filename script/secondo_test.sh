#!/bin/bash

# avvio il server
valgrind --leak-check=full ./smain ./file_config.txt ./log_file.txt

# mi salvo il pid ($! viene sostituito col PID del processo più recente avviato in background)
pid=$!

# avvio il client
./cl -f socket -p -t 1000 -d TestDirectory/output/Client/salvati -D TestDirectory/output/Client/flushati -W TestDirectory/tests/a/b/foto.jpg,TestDirectory/tests/a/b/test3.txt -w TestDirectory/tests/a/b -R 3 -c TestDirectory/tests/a/b/foto.jpg -W TestDirectory/tests/a/test.txt -r TestDirectory/tests/a/test.txt &
sleep 1
./cl -f socket -p -t 1250 -d TestDirectory/output/Client/salvati -W TestDirectory/tests/a/b/foto.jpg,TestDirectory/tests/a/b/test3.txt -w TestDirectory/tests/a/c -R 3 -c TestDirectory/tests/a/c/test6.txt -W TestDirectory/tests/a/test.txt -r TestDirectory/tests/a/test.txt &
sleep 1
./cl -f socket -p -t 1000 -d TestDirectory/output/Client/salvati -D TestDirectory/output/Client/flushati -W TestDirectory/tests/a/test2.txt -c TestDirectory/tests/a/test2.txt -W TestDirectory/tests/a/test2.txt,TestDirectory/tests/a/test.txt -R 0 -w TestDirectory/tests/a/d/ &
sleep 1
./cl -f socket -p -t 1250 -d TestDirectory/output/Client/salvati -D TestDirectory/output/Client/flushati -W TestDirectory/tests/a/b/foto.jpg,TestDirectory/tests/a/b/test3.txt -w TestDirectory/tests/a/b -R 3 -c TestDirectory/tests/a/b/foto.jpg -W TestDirectory/tests/a/test.txt -r TestDirectory/tests/a/test.txt &
sleep 1
./cl -f socket -p -t 1000 -d TestDirectory/output/Client/salvati -D TestDirectory/output/Client/flushati -W TestDirectory/tests/a/test.txt -W TestDirectory/tests/a/b/foto.jpg,TestDirectory/tests/a/b/test3.txt -w TestDirectory/tests/a/b -R 3 -c TestDirectory/tests/a/b/foto.jpg -r TestDirectory/tests/a/test.txt &

# terminazione lenta al server
kill -s SIGHUP $pid

# aspetto il server
wait $pid

exit 0