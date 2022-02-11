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
    int num_file;
} hashtable;

/* Tabella hash utilizzata da tutti i thread del server */
hashtable *table;

/* Lista di ordine FIFO utilizzata da tutti i thread del server per la politica di rimpiazzamento */
list_c *fifo_queue;


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
 * @brief Aggiunge un elemento alla tabella hash
 * 
 * @param name_file Path dell'elemento da aggiungere
 * @return int 0 in caso di successo, -1 altrimenti
 */
int add_hashtable(char *name_file);

/**
 * @brief Elimina un elemento dalla tabella hash
 * 
 * @param name_file Path dell'elemento da aggiungere 
 * @param just_deleted Nodo in cui salvare l'elemento appena eliminato
 * @return int 0 in caso di successo, -1 altrimenti
 */
int del_hashtable(char *name_file, node *just_deleted);

/**
 * @brief Cerca un nodo
 * 
 * @param name_file Path del nodo da cercare
 * @return node* Il nodo ritrovato in caso di successo, -1 altrimenti
 */
node* look_for_node(char* name_file);

#endif