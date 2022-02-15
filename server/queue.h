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
 * @brief Aggiunge un file alla lista di trabocco
 * 
 * @param lista_trabocco Lista di trabocco in cui aggiungere il file
 * @param name_file Path del file da aggiungere
 * @param flags Flag che indica se creare il file e/o settare la lock
 * @return int 0 in caso di successo, -1 altrimenti
 */
int add(list_t **lista_trabocco, char* name_file, int flags);

/**
 * @brief Rimuove un elemento dalla lista di trabocco
 * 
 * @param lista_trabocco Lista di trabocco in cui rimuovere il file
 * @param name_file Path del file da rimuovere
 * @param just_deleted Nodo in cui salvare il file appena rimosso
 * @return node Il nodo appena eliminato
 */
node* delete(list_t **lista_trabocco, char* name_file);

/**
 * @brief Setta la mutex del nodo
 * 
 * @param nodo Path del nodo di cui settare la mutex
 * @return int 0 in caso di successo, -1 altrimenti
 */
int set_mutex(node *nodo);

/**
 * @brief Resetta la mutex del nodo
 * 
 * @param nodo Path del nodo di cui resettare la mutex
 * @return int 0 in caso di successo, -1 altrimenti
 */
int unset_mutex(node *nodo);

#endif