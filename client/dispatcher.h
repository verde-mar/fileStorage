/**
 * @file dispatcher.h
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Contiene l'header della funzione implementata in dispatcher.c
 * @version 0.1
 * 
 */
#ifndef DISPATCHER_H_
#define DISPATCHER_H_

/**
 * @brief Effettua il parsing della riga di comando e chiama le API necessarie
 * 
 * @param argc Numero di argomenti
 * @param argv Argomenti della riga di comando
 * @return int In caso di successo restituisce 0, -1 altrimenti
 */
int dispatcher(int argc, char* argv[]);

#endif // !DISPATCHER_H_