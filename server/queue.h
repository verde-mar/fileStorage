#ifndef QUEUE_H_
#define QUEUE_H_

#include <pthread.h>

/**
 * @brief Nodo di ciascuna lista di trabocco
 * 
 */
typedef struct node {
    const char* path;
    char *buffer;
    int open;
    struct node* next;
    pthread_mutex_t *mutex;
    pthread_cond_t *locked;
    int fd_c;
} node;

/**
 * @brief Lista di trabocco
 * 
 */
typedef struct list {
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
 * @brief Aggiunge un elemento alla lista di trabocco specificata
 * 
 * @param lista_trabocco Lista di trabocco a cui aggiungere un elemento
 * @param name_file Path del nodo da aggiungere
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param flags Flag che indica l'operazione da eseguire (se una create e/o una lock)
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             404 nel caso in cui i flag passati non siano validi
 *             101 nel caso in cui il file esista gia'
 */
int add(list_t **lista_trabocco, char* name_file, int fd, int flags);

/**
 * @brief Elimina un nodo dalla tabella hash
 * 
 * @param lista_trabocco Lista di trabocco in cui si trova il nodo da eliminare
 * @param name_file Path del nodo da eliminare
 * @param just_deleted Nodo in cui salvare il nodo appena eliminato
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui si si cerchi  di effettuare l'operazione ma il file e' chiuso
 *             202 nel caso in cui un altro client detenga la lock
 *             505 nel caso in cui il file non esista
 */
int delete(list_t **lista_trabocco, char* name_file, node** just_deleted, int fd);

/**
 * @brief Ricerca un nodo
 * 
 * @param lista_trabocco Lista di trabocco in cui si trova il nodo
 * @param name_file Path del nodo ricercato
 * @return node* Nodo eliminato se trovato, altrimenti NULL
 */
node* look_for_node(list_t *lista_trabocco, char* name_file);

/**
 * @brief Setta il flag 'open' di un preciso nodo a 0
 * 
 * @param lista_trabocco Lista in cui si trova il nodo
 * @param name_file Path del file ricercato
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si riprovi a fare la close dopo averla gia' fatta
 *             505 nel caso in cui il file non esista
 */
int close(list_t **lista_trabocco, char* name_file, int fd);

/**
 * @brief Resetta la variabile fd_c del nodo identificato da name_file
 * 
 * @param lista_trabocco Lista di trabocco del nodo
 * @param name_file Path del nodo 
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si riprovi a fare la close dopo averla gia' fatta
 *             505 nel caso in cui il file non esista
 */
int unlock(list_t **lista_trabocco, char* name_file, int fd);

/**
 * @brief Setta la variabile fd_c del nodo identificato da name_file
 * 
 * @param lista_trabocco Lista di trabocco del nodo
 * @param name_file Path del nodo
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si riprovi a fare la close dopo averla gia' fatta
 *             505 nel caso in cui il file non esista
 */
int lock(list_t **lista_trabocco, char* name_file, int fd);

/**
 * @brief Funzione ausiliaria alla funzione lock
 * 
 * @param nodo Nodo di cui settare la mutex
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo, -1 altrimenti
 */
int set_mutex(node *nodo, int fd);

/**
 * @brief Funzione ausiliaria alla funzione unlock
 * 
 * @param nodo Nodo di cui resettare la mutex
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo, -1 altrimenti
 */
int set_unmutex(node *nodo, int fd);

#endif