#ifndef HASH_H_
#define HASH_H_

#include <queue.h>

/**
 * @brief Tabella hash
 * 
 */
typedef struct hashtable {
    struct list_t **queue;
    int num_file;
} hashtable;

/* Tabella hash utilizzata da tutti i thread del server */
hashtable *table;

/* Lista di ordine FIFO utilizzata da tutti i thread del server per la politica di rimpiazzamento */
struct list_t *fifo_queue;

/* Numero massimo di file */
int max_file;



/**
 * @brief Libera la memoria occupata dalla tabella hash
 * 
 * @return int, 0 in caso di successo, -1 altrimenti
 */
int destroy_hashtable ();

#endif