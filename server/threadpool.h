#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <pthread.h>
#include <stdio.h>
#include <fifo.h>

/**
 * @brief Risposta da inviare al client
 */
typedef struct risposta {
    int fd_richiesta;           //File descriptor del client a cui rispondere
    int errore;                 //Codice di errore da inviare al client
    void* buffer_file;          //Eventuale buffer associato al file da inviare al client
    size_t size_file;           //Size di buffer_file
} response;

/**
 *  @brief Threadpool
 */
typedef struct threadpool {
    pthread_t* threads;         //Array di workers
    int response_pipe;          //File descriptor della pipe su cui inviare le risposte al main
    list_c *pending_requests;   //Coda delle richieste pendenti
} threadpool_t;

threadpool_t *threadpool;

int create_threadpool(int num_thread);

#endif