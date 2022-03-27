#include <fifo_req.h>
#include <stdio.h>
#include <stdlib.h>

#include <check_errors.h>

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
