#include <fifo.h>
#include <stdio.h>
#include <check_errors.h>

#include <string.h>
#include <stdlib.h>

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
    node_c *tmp = NULL;
    while ((*queue)->head) {
        tmp = (*queue)->head;
        (*queue)->head = ((*queue)->head)->next;

        free((char*)tmp->path);
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

int add_fifo(char *name_file){
    node_c *current, *new_node; 
    /* Crea il nodo da aggiungere */
    new_node = malloc(sizeof(node_c));
    CHECK_OPERATION(new_node == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.La coda e' piena.\n");
            return -1);

    new_node->path = name_file;
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

int push_queue(char *request, list_c **queue){
    node_c *current, *new_node; 
    /* Crea il nodo da aggiungere */
    new_node = malloc(sizeof(node_c));
    CHECK_OPERATION(new_node == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    new_node->path = request;
    new_node->next = NULL;

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

int del(char *name_file){
    node_c* curr, *prev;
    PTHREAD_LOCK(fifo_queue->mutex);

    /* Verifica se il nodo cercato e' il primo, se e' cosi' lo elimina subito */
    curr = fifo_queue->head;
    if (strcmp(curr->path, name_file) == 0){
        fifo_queue->head = curr->next; 
        free((char*)curr->path);
        free(curr);
        fifo_queue->elements--;
        PTHREAD_UNLOCK(fifo_queue->mutex); 

        return 0;
    }
    /* Se non e' il primo, cerca in tutta la lista l'elemento, ed eventualmente lo elimina */
    prev = curr;
    curr = curr->next;
    while (curr != NULL) {
        if (strcmp(curr->path, name_file) == 0){
            prev->next = curr->next; 
            fifo_queue->elements--;
            free(curr);
            PTHREAD_UNLOCK(fifo_queue->mutex);

            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    PTHREAD_UNLOCK(fifo_queue->mutex);

    return -1;
}

char* remove_fifo(list_c *queue){
    char* name;
    node_c *temp;

    /* Elimina la testa della lista */
    PTHREAD_LOCK(queue->mutex);

    temp = queue->head;
    node_c *current = queue->head;
    queue->head = current->next;

    /* Restituisce il path del nodo appena eliminato */
    name = (char*)temp->path;

    free(temp);
    queue->elements--;

    PTHREAD_UNLOCK(queue->mutex);
    
    return name;
}

char* pop_queue(list_c *queue){
    char* name;
    node_c *temp;

    /* Elimina la testa della lista */
    PTHREAD_LOCK(queue->mutex);

    while(queue->elements == 0)
        PTHREAD_COND_WAIT(queue->cond, queue->mutex);

    temp = queue->head;
    node_c *current = queue->head;
    queue->head = current->next;

    /* Restituisce il path del nodo appena eliminato */
    name = (char*)temp->path;
    printf("NOME DELLA RICHIESTA: %s\n", name);

    free(temp);
    queue->elements--;

    PTHREAD_UNLOCK(queue->mutex);
    
    return name;
}