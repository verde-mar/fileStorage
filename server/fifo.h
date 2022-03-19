#ifndef FIFO_H_
#define FIFO_H_

#include <pthread.h>

/**
 * @brief Nodo della lista per il rimpiazzamento dei file
 * 
 */
typedef struct node_c {
    const char* path;
    struct node_c* next;
} node_c;

/**
 * @brief Lista per il rimpiazzamento dei file con ordinamento FIFO
 * 
 */
typedef struct l {
    int elements;           
    struct node_c* head;    
    pthread_mutex_t *mutex; 
    pthread_cond_t *cond;
    int how_many_cache; 
} list_c;

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

/* Lista di ordine FIFO utilizzata da tutti i thread del server per la politica di rimpiazzamento */
list_c *fifo_queue;

/**
 * @brief Crea la lista con ordinamento FIFO
 * 
 * @return int int 0 in caso di successo, -1 altrimenti
 */
int create_fifo();

/**
 * @brief Elimina la lista con ordinamento FIFO
 * 
 * @param queue Coda da eliminare
 * @return int 0 in caso di successo, -1 altrimenti
 */
int delete_fifo(list_c **queue);

/**
 * @brief Aggiunge un elemento nella coda FIFO
 * 
 * @param file_path Path del file da aggiungere
 * @return int 0
 */
int add_fifo(char *file_path);

/**
 * @brief Rimuove l' elemento identificato dal path della coda
 * 
 * @param file_path Path dell'elemento da rimuovere
 * @return int 0 in caso di successo, -1 altrimenti
 */
int del(char *file_path);

/**
 * @brief Restituisce il path dell' elemento in testa alla coda cache
 * 
 * @param queue Coda cache
 * @return Path dell'elemento in testa
 */
char* head_name(list_c *queue);

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
 * @param request Richiesta da aggiungere
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
 * @param queue Coda da cui prelevare l'elemento
 * @return request* Elemento prelevato
 */
request* pop_queue(lista_richieste *queue);

#endif