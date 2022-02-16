#ifndef HASH_H_
#define HASH_H_

#include <queue.h>
#include <fifo.h>

/**
 * @brief Tabella hash
 * 
 */
typedef struct hashtable {
    struct list **queue;
} hashtable;

/* Tabella hash utilizzata da tutti i thread del server */
hashtable *table;


/* Numero massimo di file */
int max_file;

/**
 * @brief Crea la tabella hash
 * 
 * @param size Size massima della tabella hash
 * @return int 0 in caso di successo, -1 altrimenti
 */
int create_hashtable(size_t size);

/**
 * @brief Libera la memoria occupata dalla tabella hash
 * 
 * @return int 0 in caso di successo, -1 altrimenti
 */
int destroy_hashtable ();

/**
 * @brief Crea/setta la lock di un nodo alla tabella hash
 * 
 * @param name_file Path del file da creare e/o a cui settare la lock del nodo
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param flags Flag che indicano se creare e/o settare la lock del nodo
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             404 nel caso in cui i flag passati non siano validi
 *             101 nel caso in cui il file esista gia'
 */
int add_hashtable(char *name_file, int fd, int flags);

/**
 * @brief Eliminare un nodo dalla tabella hash
 * 
 * @param name_file Path del file da eliminare
 * @param just_deleted Nodo in cui memorizzare 
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui si si cerchi  di effettuare l'operazione ma il file e' chiuso
 *             202 nel caso in cui un altro client detenga la lock
 */
int del_hashtable(char *name_file, node *just_deleted, int fd);

/**
 * @brief Setta il flag open del nodo identificato da name_file
 * 
 * @param name_file Path del file
 * @param fd File descriptor del client che ha effettuato la richiesta 
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si riprovi a fare la close dopo averla gia' fatta
 */
int close_hashtable(char *name_file, int fd);

/**
 * @brief Resetta la variabile fd_c del nodo identificato da name_file
 * 
 * @param name_file Path del file
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si riprovi a fare la close dopo averla gia' fatta
 */
int unlock_hashtable(char *name_file, int fd);

/**
 * @brief Setta la variabile fd_c del nodo identificato da name_file
 * 
 * @param name_file Path del file
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si riprovi a fare la close dopo averla gia' fatta
 */
int lock_hashtable(char *name_file, int fd);


#endif