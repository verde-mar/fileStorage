#ifndef QUEUE_H_
#define QUEUE_H_

#include <pthread.h>
#include <sys/select.h>
#include <stdio.h>

#include <fifo_req.h>
#include <fifo_cache.h>
#include <fifo_wait.h>

#include <stdatomic.h>

/**
 * @brief Nodo di ciascuna lista di trabocco
 * 
 */
typedef struct node {
    fd_set open;
    int fd_create_open;
    int fd_c;
    const char* path;
    void *buffer;
    size_t size_buffer;
    pthread_mutex_t *mutex;
    struct node* next;
    clients_in_wait *waiting_list;
} node;

/**
 * @brief Lista di trabocco
 * 
 */
typedef struct list_hashtable {
    int elements;           
    struct node* head;    
    pthread_mutex_t *mutex;  
} list_t;

/**
 * @brief Crea una lista di trabocco
 * 
 * @param lista_trabocco Lista di trabocco appena creata
 * @return int 0 in caso di successo, -1 altrimenti
 */
int create_list(list_t **lista_trabocco);

/**
 * @brief Distrugge la lista di trabocco
 * 
 * @param lista_trabocco Lista di trabocco da aggiungere
 * @return int 0 in caso di successo, -1 altrimenti
 */
int destroy_list(list_t **lista_trabocco);

/**
 * @brief Crea un nodo all'interno del server 
 * 
 * @param lista_trabocco Lista di trabocco in cui inserire il nodo
 * @param file_path Path del file da inserire
 * @param fd File descriptor del client che lo vuole inserire
 * @param max_file_reached Massimo numero di file nel server
 * @param file_log File di log
 * @return int, -1 in caso di generico fallimento
 *              101 nel caso in cui il nodo sia gia' presente
 *              0 in caso di successo
 *              
 */
int creates(list_t **lista_trabocco, char* file_path, int fd, atomic_int *max_file_reached, FILE* file_log);

/**
 * @brief Crea un nodo all'interno del server e ne acquisisce la lock
 * 
 * @param lista_trabocco Lista di trabocco in cui inserire il nodo
 * @param file_path Path del file da inserire
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param max_file_reached Massimo numero di file nel server
 * @param file_log File di log
 * @return int, -1 in caso di generico fallimento
 *              101 nel caso in cui il nodo sia gia' presente
 *              0 in caso di successo
 */
int creates_locks(list_t **lista_trabocco, char* file_path, int fd, atomic_int *max_file_reached, FILE* file_log);

/**
 * @brief Apre e acquisisce la lock di un nodo
 * 
 * @param lista_trabocco Lista di trabocco in cui si trova il nodo
 * @param file_path Path del nodo da aprire e di cui acquisire la lock
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param file_log File di log
 * @return int, -1 in caso di generico fallimento
 *              505 nel caso in cui il nodo non esista
 *              0 in caso di successo
 *              1 nel caso il client fd finisca in lista d'attesa
 */
int opens_locks(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log);

/**
 * @brief Apre un nodo
 * 
 * @param lista_trabocco Lista di trabocco in cui si trova il nodo
 * @param file_path Path del nodo da aprire 
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param file_log File di log
 * @return int, -1 in caso di generico fallimento
 *              505 nel caso in cui il nodo non esista
 *              0 in caso di successo
 */
int opens(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log);

/**
 * @brief Elimina un nodo dalla tabella hash
 * 
 * @param lista_trabocco Lista di trabocco in cui si trova il nodo da eliminare
 * @param file_path Path del nodo da eliminare
 * @param just_deleted Nodo in cui salvare il nodo appena eliminato
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param curr_size Size corrente nella tabella hash
 * @param file_log File di log
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             333 nel caso in cui il nodo cercato non esista, quindi la lista di trabocco sia null
 */
int deletes(list_t **lista_trabocco, char* file_path, node** just_deleted, int fd, atomic_int* curr_size, FILE* file_log);

/**
 * @brief Ricerca un nodo
 * 
 * @param lista_trabocco Lista di trabocco in cui si trova il nodo
 * @param file_path Path del nodo ricercato
 * @return node* Nodo trovato, altrimenti NULL
 */
node* look_for_node(list_t **lista_trabocco, char* file_path);

/**
 * @brief Setta il flag 'open' del nodo identificato da file_path a 0
 * 
 * @param lista_trabocco Lista in cui si trova il nodo
 * @param file_path Path del file ricercato
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param file_log File di log
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui il nodo non sia aperto
 *             505 nel caso in cui il file non esista
 */
int closes(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log);

/**
 * @brief Resetta la variabile fd_c del nodo identificato da file_path
 * 
 * @param lista_trabocco Lista di trabocco del nodo
 * @param file_path Path del nodo 
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param fd_next File descriptor del client appena risvegliato a cui e' stata appena data la lock
 * @param file_log File di log
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock non sia stata acquisita
 *             303 nel caso in cui si provi a fare la unlockFile dopo la closeFile
 *             505 nel caso in cui il file non esista
 *             555 nel caso in cui la lock non sia stata acquisita da nessuno
 */
int unlock(list_t **lista_trabocco, char* file_path, int fd, int *fd_next, FILE* file_log);

/**
 * @brief Setta la variabile fd_c del nodo identificato da file_path
 * 
 * @param lista_trabocco Lista di trabocco del nodo
 * @param file_path Path del nodo
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param file_log File di log
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             1 nel caso in cui la lock sia gia' occupata e il client fd venga messo in lista d'attesa
 *             505 nel caso in cui il file non esista
 */
int lock(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log);


/**
 * @brief Effettua l'append di un buffer su quello del nodo identificato da file_path
 * 
 * @param lista_trabocco Lista in cui si trova il nodo su cui effettuare l'operazione
 * @param file_path Path che identifica il nodo
 * @param buf Buffer su cui effettuare la append
 * @param size_buf Size di buf
 * @param curr_size Size corrente della tabella hash
 * @param deleted Nodo in cui memorizzare quello appena eliminato
 * @param max_size_reached Massima size raggiunta fino a quel momento
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param file_log File di log
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui si provi a fare la appendFile dopo la closeFile
 *             505 nel caso in cui il file non esista
 *             202 nel caso in cui la lock non sia stata acquisita
 */
int append_buffer(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, atomic_int* curr_size, atomic_int* max_size_reached, int fd, FILE* file_log);

/**
 * @brief Effettua la write sul buffer del nodo identificato da file_path
 * 
 * @param lista_trabocco Lista in cui si trova il nodo su cui effettuare l'operazione
 * @param file_path Path che identifica il nodo
 * @param buf Buffer da scrivere sul nodo
 * @param size_buf Size di buf
 * @param max_size Massima size possibile della tabella hash
 * @param curr_size Size corrente della tabella hash
 * @param deleted Nodo in cui memorizzare quello appena eliminato
 * @param max_size_reached Massima size raggiunta fino a quel momento
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param file_log File di log
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             505 nel caso in cui il file non esista
 *             606 nel caso in cui non sia stata fatta la openFile(O_CREATE | O_LOCK)
 */
int writes(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int *max_size, atomic_int* curr_size, atomic_int* max_size_reached, node** deleted, int fd, FILE* file_log);

/**
 * @brief Legge il file identificato da file_path
 * 
 * @param lista_trabocco Lista di trabocco in cui e' contenuto il file cercato
 * @param file_path Path del file
 * @param buf Buffer di dati
 * @param size_buf Size di buf
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param file_log File di log
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui il nodo sia chiuso
 *             505 nel caso in cui il file non esista
 *             202 nel caso in cui la lock non sia stata acquisita
 *             555 nel caso in cui la lock non sia stata acquisita da nessuno
 */
int reads(list_t **lista_trabocco, char* file_path, void** buf, size_t* size_buf, int fd, FILE* file_log);

/**
 * @brief Stampa gli elementi della lista di trabocco
 * 
 * @param queue Lista contenente gli elementi da stampare
 */
void print_hash(list_t *queue);

#endif