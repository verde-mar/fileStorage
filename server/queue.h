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
    int lock;
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
 * @brief Cerca un nodo in una lista di trabocco
 * 
 * @param lista_trabocco Lista di trabocco in cui specificare un nodo
 * @param name_file Path del nodo da ricercare
 * @return node Il nodo trovato
 */
node* look_for_node(list_t *lista_trabocco, char* name_file);

/**
 * @brief Aggiunge un elemento alla lista di trabocco specificata
 * 
 * @param lista_trabocco Lista di trabocco a cui aggiungere un elemento
 * @param name_file Path del nodo da aggiungere
 * @param fd File descriptor del client che ha effettuato la richiesta
 * @param flags Flag che indica l'operazione da eseguire (se una create e/o una lock)
 * @return int 0 in caso di successo
 *            -1 in caso di generico fallimento
 *             404 in caso di 
 *             101 nel caso in cui il file esista gia'
 */
int add(list_t **lista_trabocco, char* name_file, int fd, int flags);

/**
 * @brief 
 * 
 * @param lista_trabocco 
 * @param name_file 
 * @param fd 
 * @return node* 
 */
node* delete(list_t **lista_trabocco, char* name_file, int fd);

#endif