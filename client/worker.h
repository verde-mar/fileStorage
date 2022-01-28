#ifndef WORKER_H_
#define WORKER_H_

#include <time.h>

#define O_CREATE 2
#define O_LOCK 4

/* Variabile globale che indica se abilitare le stampe sullo stdout per ogni operazione*/
int printer;

/* Identificativo del socket file che il client usa per comunicare con il server */
int fd_skt;

/* Nome del socket file */
const char *socketname;

/**
 * @brief Viene aperta una connessione al socket file sockname
 * 
 * @param sockname Nome del socket file
 * @param msec Numero di millisecondi dopo cui il client riprova a instaurare una connessione con il server
 * @param abstime Tempo assoluto entro cui il client prova a instaurare una connessione con il server
 * @return int Restituisce 0 in caso di successo, -1 in caso di fallimento
 */
int openConnection(const char* sockname, int msec, const struct timespec abstime);

/**
 * @brief Chiude la connessione associata al socket file sockname
 * 
 * @param sockname Nome del socket file
 * @return int Restituisce 0 in caso di successo, -1 in caso di fallimento
 */
int closeConnection(const char* sockname);

/**
 * @brief DAFARE
 * 
 * @param pathname 
 * @param flags 
 * @return int 
 */
int openFile(const char *pathname, int flags);

#endif // WORKER_H_
