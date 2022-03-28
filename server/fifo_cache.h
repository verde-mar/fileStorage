#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>

/**
 * @brief Nodo della lista per il rimpiazzamento dei file
 * 
 */
typedef struct node_c {
    const char* path;
    struct node_c* next;
} node_c;

/**
 * @brief Lista per il rimpiazzamento dei file con ordinamento FIFO
 * 
 */
typedef struct l {
    int elements;           
    struct node_c* head;    
    pthread_mutex_t *mutex; 
    int how_many_cache; 
} list_cache;

/* Lista di ordine FIFO utilizzata da tutti i thread del server per la politica di rimpiazzamento */
list_cache *fifo_queue;

/**
 * @brief Crea la lista con ordinamento FIFO
 * 
 * @return int int 0 in caso di successo, -1 altrimenti
 */
int create_fifo();

/**
 * @brief Elimina la lista con ordinamento FIFO
 * 
 * @param queue Coda da eliminare
 * @return int 0 in caso di successo, -1 altrimenti
 */
int delete_fifo(list_cache **queue);

/**
 * @brief Aggiunge un elemento nella coda FIFO
 * 
 * @param file_path Path del file da aggiungere
 * @return int 0
 */
int add_fifo(char *file_path);

/**
 * @brief Rimuove l' elemento identificato dal path della coda
 * 
 * @param file_path Path dell'elemento da rimuovere
 * @return int 0 in caso di successo, -1 altrimenti
 */
int del(char *file_path);

/**
 * @brief Restituisce il path dell' elemento in testa alla coda cache
 * 
 * @param queue Coda cache
 * @return Path dell'elemento in testa
 */
char* head_name(list_cache *queue);

#endif