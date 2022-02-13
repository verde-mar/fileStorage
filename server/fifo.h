#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>

/**
 * @brief Nodo della lista FIFO
 * 
 */
typedef struct node {
    const char* path;
    struct node_c* next
} node_c;

/**
 * @brief Lista con ordinamento FIFO
 * 
 */
typedef struct list {
    int elements;           
    struct node_c* head;    
    pthread_mutex_t *mutex;  
} list_c;

/* Lista di ordine FIFO utilizzata da tutti i thread del server per la politica di rimpiazzamento */
list_c *fifo_queue;

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
 * @return int 0
 */
int add_fifo(char *name_file);

/**
 * @brief Rimuove l' elemento identificato dal path della coda
 * 
 * @param name_file Nome dell'elemento da rimuovere
 * @return int 0 in caso di successo, -1 altrimenti
 */
int remove(char *name_file);

/**
 * @brief Rimuove l' elemento in testa della coda
 * 
 * @return int 0
 */
int remove_fifo();

#endif