
#include <fifo_cache.h>
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

    return 333;
}

char* head_name(list_cache *queue){
    char *name = NULL;
    if(queue->head)
        name = (char*)(queue->head)->path;
    
    return name;
}

node_c* del_head(){
    
    node_c *tmp = fifo_queue->head;
    if(tmp)
        fifo_queue->head = (fifo_queue->head)->next;
    
    return tmp;
}