#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <pthread.h>
#include <stdio.h>
#include <fifo.h>

/**
 * @brief Risposta da inviare al client
 */
typedef struct risposta {
    int fd_richiesta;          
    int errore;                
    char* buffer_file; 
    char* path;         
} response;

/**
 *  @brief Threadpool
 */
typedef struct threadpool {
    pthread_t* threads;         
    int response_pipe;         
    list_c *pending_requests;   
} threadpool_t;

threadpool_t *threadpool;

int create_threadpool(threadpool_t **threadpool, int num_thread, int pipe_lettura);

#endif