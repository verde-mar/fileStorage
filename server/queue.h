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
 * @brief Lista linkata
 * 
 */
typedef struct list {
    int elements;           
    struct node* head;    
    pthread_mutex_t *mutex;  
} list_t;

#endif