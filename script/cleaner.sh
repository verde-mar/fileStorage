#!/bin/bash

# Ripulisce le directory utilizzate ed elimina gli eseguibili

if [ -d "./read" ]
then
	if [ "$(ls -A $DIR)" ]; then
        rm ./read/*
	fi
fi

if [ -d "./flushed" ]
then
	if [ "$(ls -A $DIR)" ]; then
        rm ./flushed/*
	fi
fi

rm ./server/*.o ./server/*.so smain ./utils/*.o socket ./client/*.o ./client/*.so cl 

clear