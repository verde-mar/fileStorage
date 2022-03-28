#ifndef FIFO_REQ_H_
#define FIFO_REQ_H_

#include <pthread.h>

/**
 * @brief Lista delle richieste con ordinamento FIFO
 * 
 */
typedef struct list_req {
    int elements;           
    struct richiesta* head;    
    pthread_mutex_t *mutex; 
    pthread_cond_t *cond;
} lista_richieste;

/**
 * @brief Richiesta inviata dal client
 */
typedef struct richiesta {
    char* request;
    void *buffer;
    size_t size_buffer;
    int fd;   
    struct richiesta* next;
} request;

/**
 * @brief Crea la lista delle richieste
 * 
 * @param queue Coda creata
 * @return int 0 in caso di successo, -1 in caso di fallimento
 */
int create_req(lista_richieste **queue);

/**
 * @brief Elimina la lista delle richieste
 * 
 * @param queue Coda da eliminare
 * @return int 0 in caso di successo, -1 in caso di fallimento
 */
int del_req(lista_richieste ** queue);

/**
 * @brief Aggiunge un elemento alla coda condivisa tra il thread main e gli worker, in mutua esclusione
 * 
 * @param req_path Richiesta da aggiungere
 * @param fd_c File descriptor del client che ha richiesto l'operazione
 * @param buffer Buffer associato alla richiesta se diverso da NULL
 * @param size_buffer Size di buffer
 * @param queue Coda a cui aggiungere un elemento
 * @return int 0 in caso di successo, -1 altrimenti
 */
int push_queue(char* req_path, int fd_c, void* buffer, size_t size_buffer, lista_richieste **queue);

/**
 * @brief Preleva un elemento in testa alla lista delle richieste
 * 
 * @param queue Lista richieste da cui prelevare l'elemento
 * @return request* Elemento prelevato
 */
request* pop_queue(lista_richieste *queue);

#endif