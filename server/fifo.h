#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>

/**
 * @brief Nodo della coda FIFO
 * 
 */
typedef struct node {
    const char* path;
} node_c;

/**
 * @brief Lista FIFO per il rimpiazzamento dei
 * 
 */
typedef struct list {
    int elements;           
    struct node_c* tail;    
    pthread_mutex_t *mutex;  
} list_c;

#endif