#ifndef QUEUE_H_
#define QUEUE_H_

#include <pthread.h>

/**
 * @brief Nodo di ciascuna lista linkata
 * 
 */
typedef struct node {
    const char* path;
    char *buffer;
    int open;
    struct node* next;
    struct node* next_cache;
    pthread_mutex_t *mutex;
} node;

/**
 * @brief Lista di trabocco
 * 
 */
typedef struct list {
    int elements;           
    struct node* tail;    
    pthread_mutex_t *mutex;  
} list_t;

/**
 * @brief Create a list object
 * 
 * @param lista_trabocco 
 * @return int 
 */
int create_list(list_t **lista_trabocco);

/**
 * @brief 
 * 
 * @param lista_trabocco 
 * @return int 
 */
int destroy_list(list_t **lista_trabocco);

#endif