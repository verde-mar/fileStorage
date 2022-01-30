#ifndef QUEUE_H_
#define QUEUE_H_

#include <pthread.h>

/**
 * @brief Nodo di ciascuna lista linkata
 * 
 */
struct node {
    const char* path;
    char *buffer;
    int open;
    struct node* next;
    struct node* next_cache;
    pthread_mutex_t *mutex;
};

/**
 * @brief Lista linkata
 * 
 */
struct list {
    int elements;           
    struct node* head;      
};

#endif