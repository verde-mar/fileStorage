#ifndef QUEUE_H_
#define QUEUE_H_

#include <pthread.h>


/**
 * @brief Nodo di ciascuna lista di trabocco
 * 
 */
typedef struct node {
    int open;
    int fd_c;
    const char* path;
    void *buffer;
    size_t size_buffer;
    pthread_mutex_t *mutex;
    pthread_cond_t *locked;
    struct node* next;
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
 * @brief Aggiunge nodo
 * 
 * @param lista_trabocco Lista di trabocco a cui aggiungere il nodo
 * @param file_path Path del nodo da aggiungere
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param flags Flag che indica l'operazione da eseguire (se una create e/o una lock)
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si provi a fare la lock dopo la close
 *             505 nel caso in cui il file non esista
 *             404 nel caso in cui i flag passati non siano validi
 *             101 nel caso in cui il file esista gia' e si e' richiesta la sola creazione di esso
 */
int add(list_t **lista_trabocco, char* file_path, int fd, int flags);

/**
 * @brief Elimina un nodo dalla tabella hash
 * 
 * @param lista_trabocco Lista di trabocco in cui si trova il nodo da eliminare
 * @param file_path Path del nodo da eliminare
 * @param just_deleted Nodo in cui salvare il nodo appena eliminato
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param curr_size Size corrente nella tabella hash
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui si si cerchi di fare la removeFile dopo la closeFile
 *             202 nel caso in cui un altro client detenga la lock
 *             505 nel caso in cui il file non esista
 */
int deletes(list_t **lista_trabocco, char* file_path, node** just_deleted, int fd, int* curr_size);

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
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si riprovi a fare la closeFile dopo averla gia' fatta
 *             505 nel caso in cui il file non esista
 */
int closes(list_t **lista_trabocco, char* file_path, int fd);

/**
 * @brief Resetta la variabile fd_c del nodo identificato da file_path
 * 
 * @param lista_trabocco Lista di trabocco del nodo
 * @param file_path Path del nodo 
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si provi a fare la unlockFile dopo la closeFile
 *             505 nel caso in cui il file non esista
 */
int unlock(list_t **lista_trabocco, char* file_path, int fd);

/**
 * @brief Setta la variabile fd_c del nodo identificato da file_path
 * 
 * @param lista_trabocco Lista di trabocco del nodo
 * @param file_path Path del nodo
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si provi a fare la lockFile dopo la closeFile
 *             505 nel caso in cui il file non esista
 */
int lock(list_t **lista_trabocco, char* file_path, int fd);


/**
 * @brief Effettua l'append di un buffer su quello del nodo identificato da file_path
 * 
 * @param lista_trabocco Lista in cui si trova il nodo su cui effettuare l'operazione
 * @param file_path Path che identifica il nodo
 * @param buf Buffer su cui effettuare la append
 * @param size_buf Size di buf
 * @param max_size Massima size possibile della tabella hash
 * @param curr_size Size corrente della tabella hash
 * @param deleted Nodo in cui memorizzare quello appena eliminato
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock sia stata acquisita da un altro client
 *             303 nel caso in cui si provi a fare la appendFile dopo la closeFile
 *             505 nel caso in cui il file non esista
 */
int append_buffer(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int* max_size, int* curr_size, int fd);

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
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *              303 nel caso in cui si provi a fare la append dopo la close
 *              505 nel caso in cui il file non esista
 *              202 nel caso in cui la lock sia stata acquisita da un altro thread
 *              444 nel caso in cui i dati del file da scrivere siano troppi 
 *              909 nel caso in cui sia stato eliminato un file
 *              808 nel caso in cui sia gia' stata fatta la writeFile su quel nodo
 */
int writes(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int *max_size, int* curr_size, node** deleted, int fd);

/**
 * @brief Legge il file identificato da file_path
 * 
 * @param lista_trabocco Lista di trabocco in cui e' contenuto il file cercato
 * @param file_path Path del file
 * @param buf Buffer di dati
 * @param size_buf Size di buf
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui si provi a fare la writeFile dopo la closeFile
 *             505 nel caso in cui il file non esista
 */
int reads(list_t **lista_trabocco, char* file_path, void** buf, size_t* size_buf, int fd);

#endif