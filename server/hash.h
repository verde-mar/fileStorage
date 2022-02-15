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
 * @brief Aggiunge un elemento alla tabella hash
 * 
 * @param name_file Path dell'elemento da aggiungere
 * @param flags Flag che indica se creare il file e/o settare la lock
 * @return int 0 in caso di successo, -1 altrimenti
 */
int add_hashtable(char *name_file, int flags);

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

/**
 * @brief Setta la lock del nodo identificato dal path passato per parametro
 * 
 * @param name_file Path del file di cui settare la lock
 * @return int 0 in caso di successo, -1 altrimenti
 */
int set_lock(char* name_file);

/**
 * @brief Setta il flag 'open' del nodo identificato da name_file, a 1
 * 
 * @param name_file Path del nodo di cui settare il flag
 * @return int 0 in caso di successo, -1 altrimenti
 */
int set_flag(char* name_file);

/**
 * @brief Resetta il flag 'open' del nodo identificato da name_file
 * 
 * @param name_file Path del nodo di cui resettare il flag
 * @return int int 0 in caso di successo, -1 altrimenti
 */
int unset_lock(char* name_file);

#endif