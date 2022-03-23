/**
 * @file fifo.c
 * @author Sara Grecu (s.grecu1@studenti.unipi.it)
 * @brief Codice relativo sia alla coda FIFO delle richieste sia a quella per il rimpiazzamento dei file
 * @version 0.1
 * @date 2022-03-09
 * 
 */
#include <fifo.h>
#include <stdio.h>
#include <check_errors.h>

#include <string.h>
#include <stdlib.h>

//TODO: per usare una sola funzione si puo' fare la malloc da un'altra parte, ma poi incasinerei il binomio malloc/destroy ---> metti nella relazione

int create_fifo(){
    fifo_queue = malloc(sizeof(list_cache));
    CHECK_OPERATION(fifo_queue == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
        return -1);

    /* Inizializza il numero di elementi iniziali */
    fifo_queue->elements = 0;
    /* Inizializza la testa */
    fifo_queue->head = NULL;

    /* Inizializza la mutex */
    fifo_queue->mutex = malloc(sizeof(pthread_mutex_t));
    CHECK_OPERATION(fifo_queue->mutex == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
    PTHREAD_INIT_LOCK(fifo_queue->mutex);

    /* Inizializza il numero di volte in cui e' stato chiamato l'algoritmo di rimpiazzamento */
    fifo_queue->how_many_cache = 0;

    return 0;
}

int delete_fifo(list_cache **queue){
    CHECK_OPERATION(!(*queue), fprintf(stderr, "Parametri non validi.\n"); return -1);

    /* Rimuove ogni elemento della coda */
    while ((*queue)->head!=NULL) {
        node_c *tmp = (*queue)->head;
        (*queue)->head = ((*queue)->head)->next;
        free(tmp);
    }
    
    /* Distrugge la lock di ciascun nodo */
    PTHREAD_DESTROY_LOCK((*queue)->mutex);
    free((*queue)->mutex);
    /* Libera la memoria occupata dalla lista di trabocco */
    free((*queue));

    return 0;
}

int add_fifo(char *file_path){
    node_c *current, *new_node; 
    /* Crea il nodo da aggiungere */
    new_node = malloc(sizeof(node_c));
    CHECK_OPERATION(new_node == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.La coda e' piena.\n");
        return -1);

    new_node->path = file_path;
    new_node->next = NULL;

    /* Aggiunge il nuovo nodo in coda */
    current = fifo_queue->head;
    if (current == NULL)
        fifo_queue->head = new_node; 
    else {
        while(current->next!=NULL)
            current = current->next;
        current->next = new_node;
    }
    fifo_queue->elements++;

    return 0;
}

int del(char *file_path){
    node_c* curr, *prev;
    
    /* Verifica se il nodo cercato e' il primo, se e' cosi' lo elimina subito */
    curr = fifo_queue->head;
    if (strcmp(curr->path, file_path) == 0){
        fifo_queue->head = curr->next; 
        free(curr);
        
        fifo_queue->elements--;

        return 0;
    }
    /* Se non e' il primo, cerca in tutta la lista l'elemento, ed eventualmente lo elimina */
    prev = curr;
    curr = curr->next;
    while (curr != NULL) {
        if (strcmp(curr->path, file_path) == 0){
            prev->next = curr->next; 
            fifo_queue->elements--;
            free(curr);

            return 0;
        }
        prev = curr;
        curr = curr->next;
    }   

    return -1;
}

char* head_name(list_cache *queue){
    char *name = (char*)(queue->head)->path;

    return name;
}

int push_queue(char* req_path, int fd_c, void* buffer, size_t size_buffer, lista_richieste **queue){
    CHECK_OPERATION((*queue)==NULL, fprintf(stderr, "Parametri non validi."); return -1);

    request *current, *new_node; 

    /* Crea il nodo da aggiungere */
    new_node = malloc(sizeof(request));
    CHECK_OPERATION(new_node == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
        return -1);

    if(req_path!=NULL){
        new_node->request = req_path;
        new_node->fd = fd_c; 
        new_node->buffer = buffer;
        new_node->size_buffer = size_buffer;
        new_node->next = NULL;
    } else {
        new_node->request = NULL;
        new_node->fd = fd_c;
        new_node->next = NULL;
    }

    /* Aggiunge il nuovo nodo in coda */
    PTHREAD_LOCK((*queue)->mutex);

    current = (*queue)->head;
    if (current == NULL)
        (*queue)->head = new_node; 
    else {
        while(current->next!=NULL)
            current = current->next;
        current->next = new_node;
    }
    (*queue)->elements++;

    PTHREAD_COND_SIGNAL((*queue)->cond);

    PTHREAD_UNLOCK((*queue)->mutex);

    return 0;
}

request* pop_queue(lista_richieste *queue){
    request *temp;

    /* Elimina la testa della lista */
    PTHREAD_LOCK(queue->mutex);

    while(queue->elements == 0)
        PTHREAD_COND_WAIT(queue->cond, queue->mutex);

    temp = queue->head;
    request *current = queue->head;
    queue->head = current->next;

    /* Restituisce il path del nodo appena eliminato */
    
    queue->elements--;

    PTHREAD_UNLOCK(queue->mutex);
    
    return temp;
}

int create_req(lista_richieste **queue){ 
    *queue = malloc(sizeof(lista_richieste));
    CHECK_OPERATION((*queue) == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    /* Inizializza il numero di elementi iniziali */
    (*queue)->elements = 0;
    /* Inizializza la testa */
    (*queue)->head = NULL;
    /* Inizializza la mutex */
    (*queue)->mutex = malloc(sizeof(pthread_mutex_t));
    CHECK_OPERATION((*queue)->mutex == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
    PTHREAD_INIT_LOCK((*queue)->mutex);
    /* Inizializza la variabile di condizione */
    (*queue)->cond = malloc(sizeof(pthread_cond_t));
    CHECK_OPERATION((*queue)->cond == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
    PTHREAD_INIT_COND((*queue)->cond);

    return 0;
}

int del_req(lista_richieste **queue){
    CHECK_OPERATION(!(*queue), fprintf(stderr, "Parametri non validi.\n"); return -1);

    /* Rimuove ogni elemento della coda */
    request *tmp = NULL;
    while ((*queue)->head) {
        tmp = (*queue)->head;
        (*queue)->head = ((*queue)->head)->next;
        if(tmp->buffer)
            free(tmp->buffer);
        free(tmp->request);
        free(tmp); 
    }
    
    /* Distrugge la lock di ciascun nodo */
    PTHREAD_DESTROY_LOCK((*queue)->mutex);
    free((*queue)->mutex);
    /* Distrugge la variabile di condizione di ciascun nodo */
    PTHREAD_DESTROY_COND((*queue)->cond);
    free((*queue)->cond);
    /* Libera la memoria occupata dalla lista di trabocco */
    free((*queue));

    return 0;
}

int create_list_wait(clients_in_wait **list){
    *list = malloc(sizeof(clients_in_wait));
    CHECK_OPERATION(*list == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
        return -1);

    /* Inizializza la testa */
    (*list)->head = NULL;
    /* Inizializza il numero di elementi */
    (*list)->elements = 0;

    return 0;
}

int delete_list_wait(clients_in_wait **queue){
    CHECK_OPERATION(!(*queue), fprintf(stderr, "Parametri non validi.\n"); return -1);

    /* Rimuove ogni elemento della coda */
    while ((*queue)->head!=NULL) {
        client *tmp = (*queue)->head;
        (*queue)->head = ((*queue)->head)->next;
        free(tmp);
    }
    /* Libera la memoria occupata dalla lista di trabocco */
    free((*queue));

    return 0;
}

int add_list_wait(int file_d, clients_in_wait* list){
    CHECK_OPERATION(file_d < 0 || list == NULL, fprintf(stderr, "Parametri non validi.\n"); return -1;);

    client *current, *new_node; 
    /* Crea il nodo da aggiungere */
    new_node = malloc(sizeof(client));
    CHECK_OPERATION(new_node == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.La coda e' piena.\n");
        return -1);

    new_node->file_descriptor = file_d;
    new_node->next = NULL;

    /* Aggiunge il nuovo nodo in coda */
    current = list->head;
    if (current == NULL)
        list->head = new_node; 
    else {
        while(current->next!=NULL)
            current = current->next;
        current->next = new_node;
    }
    list->elements++;

    return 0;
}

int del_list_wait(client **head_client, clients_in_wait* list){
    CHECK_OPERATION(head_client == NULL || list == NULL, fprintf(stderr, "Parametri non validi.\n"); return -1;);
    client* curr;
    
    curr = list->head;
    *head_client = curr;
    list->head = curr->next; 
    
    list->elements--;

    return 0;
}

