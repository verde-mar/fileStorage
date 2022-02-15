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


int add_hashtable(char *name_file, int fd_c, int flags);


int del_hashtable(char *name_file, node *just_deleted, int fd);


#endif