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
} node;

/**
 * @brief Lista di trabocco
 * 
 */
typedef struct list {
    int elements;           
    struct node* tail;    
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
 * @return int 0 in caso di successo, -1 altrimenti
 */
int add(list_t **lista_trabocco, char* name_file);

/**
 * @brief Rimuove un elemento dalla lista di trabocco
 * 
 * @param lista_trabocco Lista di trabocco in cui rimuovere il file
 * @param name_file Path del file da rimuovere
 * @param just_deleted Nodo in cui salvare il file appena rimosso
 * @return int 0 in caso di successo, -1 altrimenti
 */
int delete(list_t **lista_trabocco, char* name_file, node **just_deleted);

#endif