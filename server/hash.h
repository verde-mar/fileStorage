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
    int curr_size;
    int max_size;
    int max_file;
    pthread_mutex_t *mutex_t;
} hashtable;

/* Tabella hash utilizzata da tutti i thread del server */
hashtable *table;

/**
 * @brief Funzione per calcolare l'hash
 * 
 * @param str Stringa di cui calcolare l'hash
 * @return unsigned long Hash calcolato
 */
unsigned long hash_function(unsigned char *str);

//TODO:metti una funzione unica che prende dei puntatori a funzione 

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
 *             505 nel caso in cui il file non esista
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
 *             303 nel caso in cui si provi a fare la close dopo averla gia' fatta
 *             505 nel caso in cui il file non esista
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
 *             303 nel caso in cui si provi a fare la unlock dopo la close
 *             505 nel caso in cui il file non esista
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
 *             303 nel caso in cui si provi a fare la lock dopo la close
 *             505 nel caso in cui il file non esista
 */
int lock_hashtable(char *name_file, int fd);

/**
 * @brief Legge un elemento dalla tabella hash
 * 
 * @param name_file Path del file da leggere
 * @param buf Buffer in cui memorizzare i dati del nodo 
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *             -1 in caso di generico fallimento
 *              303 nel caso in cui si provi a fare la read dopo la close
 *              505 nel caso in cui il file non esista
 */
int read_hashtable(char *name_file, char** buf, int fd);



#endif