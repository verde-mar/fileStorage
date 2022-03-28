#ifndef HASH_H_
#define HASH_H_

#include <queue.h>
#include <stdatomic.h>
/**
 * @brief Tabella hash
 * 
 */
typedef struct hashtable {
    list_t **queue;
    atomic_int curr_size;
    int max_size;
    int max_file; 
    atomic_int max_size_reached; 
    atomic_int max_file_reached; 
    FILE *file_log;
} hashtable;

/* Tabella hash utilizzata da tutti i thread del server */
hashtable *table;

/**
 * @brief Funzione per calcolare l'hash
 * 
 * @param str Stringa di cui calcolare l'hash
 * @return unsigned long Hash calcolato
 */
unsigned long hash_function(char *str);

/**
 * @brief Crea la tabella hash
 * 
 * @param size Size massima della tabella hash
 * @param num_file Numero massimo di file da memorizzare nella tabella hash
 * @param name_log_file Nome del file di log
 * @return int 0 in caso di successo, -1 altrimenti
 */
int create_hashtable(size_t size, int num_file, char *name_log_file);

/**
 * @brief Trova un file in una lista di trabocco
 * 
 * @param lista_trabocco Lista di trabocco in cui cercare il file
 * @param file_path Path del file da cercare
 * @return node* Nodo trovato in caso di successo, NULL altrimenti
 */
node* look_for_node(list_t **lista_trabocco, char* file_path);

/**
 * @brief Libera la memoria occupata dalla tabella hash
 * 
 * @return int 0 in caso di successo, -1 altrimenti
 */
int destroy_hashtable ();

/**
 * @brief Crea e acquisisce la lock su un file
 * 
 * @param path Path del file da creare e su cui acquisire la lock
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param just_deleted Nodo in cui memorizzare un eventuale file eliminato, perche' raggiunto il limite di file nello storage
 * @return int 0 in caso di successo,
 *             -1 in caso di generico fallimento
 *             101 nel caso in cui il file sia gia' presente
 */
int creates_locks_hashtable(char *path, int fd, node **just_deleted);

/**
 * @brief Crea un file nello storage
 * 
 * @param path Path del file da creare
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param just_deleted Nodo in cui memorizzare un eventuale file eliminato, perche' raggiunto il limite di file nello storage
 * @return int -1 in caso di generico fallimento
 *              101 nel caso in cui il file sia gia' presente
 *              0 in caso di successo
 */
int creates_hashtable(char *path, int fd, node **just_deleted);

/**
 * @brief Apre un file e ne acquisisce la lock
 * 
 * @param path Path del file da aprire e su cui acquisire la lock
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int -1 in caso di generico fallimento
 *              505 nel caso in cui il nodo non esista
 *              0 in caso di successo
 *              1 nel caso il client fd finisca in lista d'attesa
 */
int opens_locks_hashtable(char *path, int fd);

/**
 * @brief Apre un file
 * 
 * @param path Path del file da aprire
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int -1 in caso di generico fallimento
 *              505 nel caso in cui il nodo non esista
 *              0 in caso di successo
 */
int opens_hashtable(char *path, int fd);

/**
 * @brief Eliminare un file dalla tabella hash
 * 
 * @param file_path Path del file da eliminare
 * @param just_deleted Nodo in cui memorizzare 
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             333 nel caso in cui il nodo cercato non esista, quindi la lista di trabocco sia null
 */
int del_hashtable(char *file_path, node **just_deleted, int fd);

/**
 * @brief Setta il flag open del file identificato da file_path
 * 
 * @param file_path Path del file
 * @param fd File descriptor del client che ha effettuato la richiesta 
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui si provi a fare la close dopo averla gia' fatta
 *             505 nel caso in cui il file non esista
 */
int close_hashtable(char *file_path, int fd);

/**
 * @brief Resetta la variabile fd_c del file identificato da file_path
 * 
 * @param file_path Path del file
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param fd_next File descriptor del client risvegliato dalla lista di attesa a cui e' stata appena data la lock
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             202 nel caso in cui la lock non sia stata acquisita
 *             303 nel caso in cui si provi a fare la unlockFile dopo la closeFile
 *             505 nel caso in cui il file non esista
 *             555 nel caso in cui la lock non sia stata acquisita da nessuno
 */
int unlock_hashtable(char *path, int fd, int* fd_next);

/**
 * @brief Setta la variabile fd_c del file identificato da file_path
 * 
 * @param file_path Path del file
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             1 nel caso in cui la lock sia gia' occupata e il client fd venga messo in lista d'attesa
 *             505 nel caso in cui il file non esista
 */
int lock_hashtable(char *file_path, int fd);

/**
 * @brief Effettua l'append sul file identificato da file_path
 * 
 * @param file_path Path del file su cui eseguire l'append
 * @param buf Buffer di cui fare l'append
 * @param deleted Nodo in cui memorizzare un file eventualmente eliminato per fare spazio a buf
 * @param fd File descriptor del client che ha richiesto l'operazione 
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui si provi a fare la appendFile dopo la closeFile
 *             505 nel caso in cui il file non esista
 *             202 nel caso in cui la lock non sia stata acquisita
 */
int append_hashtable(char* file_path, void* buf, size_t* size_buf, node** deleted, int fd);

/**
 * @brief Scrive sul file identificato da file_path
 * 
 * @param file_path Path del file su cui eseguire la write
 * @param buf Buffer da scrivere sul file
 * @param deleted Nodo in cui memorizzare un file eventualmente eliminato per fare spazio a buf
 * @param fd File descriptor del client che ha richiesto l'operazione 
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             505 nel caso in cui il file non esista
 *             606 nel caso in cui non sia stata fatta la openFile(O_CREATE | O_LOCK)
 */
int write_hashtable(char* file_path, void* buf, size_t* size_buf, node** deleted, int fd);

/**
 * @brief Legge un elemento dalla tabella hash
 * 
 * @param file_path Path del file da leggere
 * @param buf Buffer in cui memorizzare i dati del file 
 * @param size_buf Size di buf
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             303 nel caso in cui il nodo sia chiuso
 *             505 nel caso in cui il file non esista
 *             202 nel caso in cui la lock non sia stata acquisita
 *             555 nel caso in cui la lock non sia stata acquisita da nessuno
 */
int read_hashtable(char *file_path, void** buf, size_t* size_buf, int fd);

/**
 * @brief Legge un elemento dalla tabella hash
 * 
 * @param N Numero di file da inviare
 * @param buf Buffer in cui memorizzare i dati del file 
 * @param size_buf Size di buf
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param path Path del file da inviare
 * @return int 0 in caso di successo
 *             -1 in caso di generico fallimento
 *              303 nel caso in cui si provi a fare la read dopo la close
 *              505 nel caso in cui il file non esista
 *              111 nel caso in cui non ci siano piu' file da leggere
 */
int readN_hashtable(int N, void** buf, size_t *size_buf, int fd, char** path);

/**
 * @brief Elimina definitivamente il file just_deleted
 * 
 * @param just_deleted Nodo da eliminare
 * @return int 0 in caso di successo, -1 altrimenti
 */
int definitely_deleted(node** just_deleted);

/**
 * @brief Stampa gli elementi della tabella hash
 * 
 */
void print_elements();

#endif