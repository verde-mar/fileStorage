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

#endif 