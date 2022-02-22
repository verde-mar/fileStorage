#include <fifo.h>
#include <stdio.h>
#include <check_errors.h>

#include <string.h>
#include <stdlib.h>

int create_fifo(){
    fifo_queue = malloc(sizeof(list_c));
    CHECK_OPERATION(fifo_queue == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    /* Inizializza il numero di elementi iniziali */
    fifo_queue->elements = 0;
    /* Inizializza la testa */
    fifo_queue->head = NULL;
    /* Inizializza la mutex */
    fifo_queue->mutex = malloc(sizeof(pthread_mutex_t));
    PTHREAD_INIT_LOCK(fifo_queue->mutex);

    return 0;
}

int delete_fifo(){
    /* Rimuove ogni elemento della coda */
    node_c *tmp = NULL;
    while (fifo_queue->head) {
        tmp = fifo_queue->head;
        fifo_queue->head = (fifo_queue->head)->next;
        free(tmp);
    }
    
    /* Distrugge la lock di ciascun nodo */
    PTHREAD_DESTROY_LOCK(fifo_queue->mutex);
    free(fifo_queue->mutex);
    /* Libera la memoria occupata dalla lista di trabocco */
    free(fifo_queue);

    return 0;
}

int add_fifo(char *name_file){
    node_c *current, *new_node; 
    /* Crea il nodo da aggiungere */
    new_node = malloc(sizeof(node_c));
    CHECK_OPERATION(new_node == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
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

int del(char *name_file){
    node_c* curr, *prev;
    PTHREAD_LOCK(fifo_queue->mutex);

    /* Verifica se il nodo cercato e' il primo, se e' cosi' lo elimina subito */
    curr = fifo_queue->head;
    if (strcmp(curr->path, name_file) == 0){
        fifo_queue->head = curr->next; 
        free(curr);
        fifo_queue->elements--;
        PTHREAD_UNLOCK(fifo_queue->mutex); //chiedi a tato se va bene che ci siano le lock nella coda fifo, anche se il metodo viene chiamato all'interno dei metodi
        //della queue.c anche quando la queue.c prende la sua di lock

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

char* remove_fifo(){
    char* name;
    node_c *temp;

    /* Elimina la testa della lista */
    PTHREAD_LOCK(fifo_queue->mutex);
    if(fifo_queue->head == NULL){
        PTHREAD_UNLOCK(fifo_queue->mutex);
        return NULL;
    }

    temp = fifo_queue->head;
    node_c *current = fifo_queue->head;
    fifo_queue->head = current->next;

    /* Restituisce il path del nodo appena eliminato */
    name = (char*)temp->path;

    free(temp);
    fifo_queue->elements--;

    PTHREAD_UNLOCK(fifo_queue->mutex);
    
    return name;
}

