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

int create_fifo(list_c **queue){
    *queue = malloc(sizeof(list_c));
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

int delete_fifo(list_c **queue){
    /* Rimuove ogni elemento della coda */
    while ((*queue)->head!=NULL) {
        node_c *tmp = (*queue)->head;
        (*queue)->head = ((*queue)->head)->next;

        free(tmp);
    }
    
    /* Distrugge la lock di ciascun nodo */
    PTHREAD_DESTROY_LOCK((*queue)->mutex, "delete_fifo: queue->mutex");
    free((*queue)->mutex);
    /* Distrugge la variabile di condizione di ciascun nodo */
    PTHREAD_DESTROY_COND((*queue)->cond);
    free((*queue)->cond);
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

char* head_name(list_c *queue){
    char *name = (char*)(queue->head)->path;
    fprintf(stdout, "Questo file sta per essere eliminato: %s\n", name);

    return name;
}

int push_queue(char* req_path, int fd_c, void* buffer, size_t size_buffer, lista_richieste **queue){
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
    /* Rimuove ogni elemento della coda */
    request *tmp = NULL;
    while ((*queue)->head) {
        tmp = (*queue)->head;
        (*queue)->head = ((*queue)->head)->next;
        free(tmp->buffer);
        free(tmp->request);
        free(tmp);
    }
    
    /* Distrugge la lock di ciascun nodo */
    PTHREAD_DESTROY_LOCK((*queue)->mutex, "del_req: queue->mutex");
    free((*queue)->mutex);
    /* Distrugge la variabile di condizione di ciascun nodo */
    PTHREAD_DESTROY_COND((*queue)->cond);
    free((*queue)->cond);
    /* Libera la memoria occupata dalla lista di trabocco */
    free((*queue));

    return 0;
}