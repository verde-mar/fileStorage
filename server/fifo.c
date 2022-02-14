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

    int mutex_init = pthread_mutex_init(fifo_queue->mutex, NULL);
    CHECK_OPERATION(mutex_init == -1,
        fprintf(stderr, "Non e' stato possibile inizializzare la mutex della lista di trabocco.\n");
            return -1);

    fifo_queue->head = NULL;
    fifo_queue->elements = 0;

    return 0;
}

int delete_fifo(){
    node_c *tmp = NULL;
    while (fifo_queue->head) {
        tmp = fifo_queue->head;
        fifo_queue->head = (fifo_queue->head)->next;
        free((char*)tmp->path);
        free(tmp);
    }
    
    int check_dest = pthread_mutex_destroy(fifo_queue->mutex);
    CHECK_OPERATION(check_dest == -1,
        fprintf(stderr, "Non e' stato possibile distruggere la mutex della lista di trabocco.\n");
            return -1);
    free(fifo_queue);

    return 0;
}

int add_fifo(char *name_file){
    node_c *current, *new_node; 
    new_node = malloc(sizeof(node_c));
    CHECK_OPERATION(new_node == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    new_node->path = name_file;
    new_node->next = NULL;

    pthread_mutex_lock(fifo_queue->mutex);
    current = fifo_queue->head;
    if (current == NULL)
        fifo_queue->head = new_node; 

    fifo_queue->elements++;

    pthread_mutex_unlock(fifo_queue->mutex);

    return 0;
}

int del(char *name_file){
    pthread_mutex_lock(fifo_queue->mutex);

    node_c* curr, *prev;
    curr = fifo_queue->head;
    if (strcmp(curr->path, name_file) == 0){
        fifo_queue->head = curr->next; 
        free(curr);
        fifo_queue->elements--;
        pthread_mutex_unlock(fifo_queue->mutex);

        return 0;
    }

    prev = curr;
    curr = curr->next;
    while (curr != NULL) {
        if (strcmp(curr->path, name_file) == 0){
            prev->next = curr->next; 
            fifo_queue->elements--;
            free(curr);
            pthread_mutex_unlock(fifo_queue->mutex);

            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    pthread_mutex_unlock(fifo_queue->mutex);

    return -1;
}


char* remove_fifo(){
    pthread_mutex_lock(fifo_queue->mutex);
    char* name;

    node_c *temp;
    temp = fifo_queue->head;
    node_c *current = fifo_queue->head;
    fifo_queue->head = current->next;
    name = malloc(sizeof(char)*(strlen(temp->path)+1));
    strcpy(name, temp->path);

    free(temp);
    fifo_queue->elements--;

    pthread_mutex_unlock(fifo_queue->mutex);
    
    return name;
}