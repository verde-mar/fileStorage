#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <pthread.h>
#include <stdio.h>
#include <hash.h>

/**
 * @brief Risposta da inviare al client
 */
typedef struct risposta {
    int fd_richiesta;          
    size_t errore;                
    char* buffer_file; 
    char* path;     
    node *deleted;    
} response;

/**
 *  @brief Threadpool
 */
typedef struct threadpool {
    pthread_t* threads;         
    int response_pipe;         
    lista_richieste *pending_requests;   
    int num_thread;
    int curr_threads;
} threadpool_t;

/**
 * @brief Crea un threadpool
 * 
 * @param threadpool Threadpool da creare
 * @param num_thread Numero di thread che ci possono essere
 * @param pipe_lettura File descriptor della pipe su cui inviare le risposte
 * @return int 0 in caso di successo, -1 altrimenti
 */
int create_threadpool(threadpool_t **threadpool, int num_thread, int pipe_scrittura);

/**
 * @brief Distrugge il threadpool
 * 
 * @param threadpool Threadpool da distruggere
 * @return int 0 in caso di successo, -1 altrimenti
 */
int destroy_threadpool(threadpool_t **threadpool);

/**
 * @brief Invia la risposta per il client al thread main
 * 
 * @param pool Threadpool
 * @param err Codice di errore generato
 * @param fd File descriptor del client che ha richiesto l'operazione
 * @param buf Eventuale buffer di dati 
 * @param path Eventuale path associato a buf
 * @param deleted Nodo in cui memorizzare un eventuale file eliminato
 * @return int 0 in caso di successo, -1 altrimenti
 */
int invia_risposta(threadpool_t *pool, int err, int fd, char* buf, char* path, node *deleted);

#endif