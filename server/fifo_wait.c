#include <fifo_wait.h>
#include <stdio.h>
#include <stdlib.h>

#include <check_errors.h>

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
    CHECK_OPERATION(list == NULL, fprintf(stderr, "Parametri non validi.\n"); return -1;);
    client* curr;
    
    curr = list->head;
    *head_client = curr;
    if(curr){
        list->head = curr->next; 
    } else {
        list->head = NULL;
    }
    list->elements--;

    return 0;
}