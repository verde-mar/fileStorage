#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>

/**
 * @brief Nodo della lista FIFO
 * 
 */
typedef struct node {
    const char* path;
} node_c;

/**
 * @brief Lista con ordinamento FIFO
 * 
 */
typedef struct list {
    int elements;           
    struct node_c* tail;    
    pthread_mutex_t *mutex;  
} list_c;

/**
 * @brief Crea la lista con ordinamento FIFO
 * 
 * @param lista_fifo Lista da allocare
 * @return int 0 in caso di successo, -1 altrimenti
 */
int create_fifo(list_c **lista_fifo);

/**
 * @brief Elimina la lista con ordinamento FIFO
 * 
 * @param lista_fifo Lista da eliminare
 * @return int 0 in caso di successo, -1 altrimenti
 */
int delete_fifo(list_c **lista_fifo);

/**
 * @brief Aggiunge un elemento nella coda FIFO
 * 
 * @param name_file Nome del file da aggiungere
 * @return int 0 in caso di successo, -1 altrimenti
 */
int add_fifo(char *name_file);

/**
 * @brief Rimuove un elemento della coda
 * 
 * @param name_file Nome dell'elemento da rimuovere
 * @return int 0 in caso di successo, -1 altrimenti
 */
int remove_fifo(char *name_file);

#endif