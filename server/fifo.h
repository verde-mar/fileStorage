#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>

/**
 * @brief Nodo della lista FIFO
 * 
 */
typedef struct node_c {
    const char* path;
    struct node_c* next;
} node_c;

/**
 * @brief Lista con ordinamento FIFO
 * 
 */
typedef struct list_cache {
    int elements;           
    struct node_c* head;    
    pthread_mutex_t *mutex; 
    pthread_cond_t *cond;
} list_c;

/* Lista di ordine FIFO utilizzata da tutti i thread del server per la politica di rimpiazzamento */
list_c *fifo_queue;

/* Lista di ordine FIFO utilizzata dal main e dai worker per la gestione delle richieste */
list_c *queue_workers;

/**
 * @brief Crea la lista con ordinamento FIFO
 * 
 * @return int 0 in caso di successo, -1 altrimenti
 */
int create_fifo();

/**
 * @brief Elimina la lista con ordinamento FIFO
 * 
 * @return int 0 in caso di successo, -1 altrimenti
 */
int delete_fifo();

/**
 * @brief Aggiunge un elemento nella coda FIFO
 * 
 * @param name_file Path del file da aggiungere
 * @return int 0
 */
int add_fifo(char *name_file);

/**
 * @brief Rimuove l' elemento identificato dal path della coda
 * 
 * @param name_file Path dell'elemento da rimuovere
 * @return int 0 in caso di successo, -1 altrimenti
 */
int del(char *name_file);

/**
 * @brief Rimuove l' elemento in testa della coda
 * 
 * @return Path dell'elemento appena eliminato
 */
char* remove_fifo();

#endif