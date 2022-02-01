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
 * @brief Richiede l'apertura di un file e/o dell' acquisizione la lock
 * 
 * @param pathname Path assoluto del file
 * @param flags Flag che decidono se aprire e/o acquisire la lock sul file
 * @return int 0 in caso di successo -1 altrimenti
 */
int openFile(const char *pathname, int flags);

/**
 * @brief Richiede l'acquisizione della lock di un file
 * 
 * @param pathname Path assoluto del file
 * @return int 0 in caso di successo -1 altrimenti
 */
int lockFile(const char* pathname);

/**
 * @brief Richiede il rilascio della lock di un file
 * 
 * @param pathname Path assoluto del file
 * @return int 0 in caso di successo -1 altrimenti
 */
int unlockFile(const char* pathname);

/**
 * @brief Richiede la rimozione di un file dal server
 * 
 * @param pathname Path assoluto del file
 * @return int 0 in caso di successo -1 altrimenti
 */
int removeFile(const char* pathname);

/**
 * @brief Richiede la chiusura di un file
 * 
 * @param pathname Path assoluto del file
 * @return int 0 in caso di successo, -1 altrimenti
 */
int closeFile(const char* pathname);

/**
 * @brief Legge un file dal server
 * 
 * @param pathname Path assoluto del file da leggere
 * @param buf Buffer in cui memorizzare i dati letti
 * @param size Size di buf
 * @return int 0 in caso di successo, -1 altrimenti
 */
int readFile(const char* pathname, void** buf, size_t *size);

/**
 * @brief Scrive un file nel server
 * 
 * @param pathname Path assoluto del file
 * @param dirname Directory in cui salvare i file eliminati dal server per fare posto a pathname
 * @return int 0 in caso di successo, -1 altrimenti
 */
int writeFile(const char* pathname, const char* dirname);

/**
 * @brief Richiesta di scrivere in append al file 'pathname' i 'size' bytes contenuti nel buffer 'buf'
 * 
 * @param pathname Path assoluto del file
 * @param buf Buffer da scrivere in append al file
 * @param size Size di buf
 * @param dirname Directory dove memorizzare il file espulso dal server
 * @return int 0 in caso di successo, -1 altrimenti
 */
int appendToFile(const char* pathname, void* buf, size_t size, const char* dirname);

#endif // WORKER_H_
