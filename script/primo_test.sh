#!/bin/bash

valgrind --leak-check=full ./smain ./file_config.txt ./log_file.txt

# mi salvo il pid ($! viene sostituito col PID del processo più recente avviato in background)
pid=$!

#chiama i client

# aspetto il termine
wait

# terminazione lenta al server
kill -s SIGHUP $pid

# aspetto il server
wait $pid

./script/statistiche.sh

exit 0