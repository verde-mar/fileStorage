#!/bin/bash

# Inizializza il file di log

echo "socketname: $1" > $5
echo "size: $3" >> $5
echo "numero di workers: $4" >> $5
echo "numero massimo di file accettabili: $2" >> $5