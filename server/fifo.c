#include <fifo.h>
#include <stdio.h>
#include <check_errors.h>

//TODO: testa
int add_fifo(char *name_file){
    node_c *current, *new_node; 
    new_node = malloc(sizeof(node_c));
    CHECK_OPERATION(new_node == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    new_node->path = name_file;
    new_node->next = NULL;
    current = fifo_queue->head;
    if (current == NULL)
        fifo_queue->head = new_node; 

    return 0;
}

//TODO: testa
int remove(char *name_file){
    if(fifo_queue->head == NULL)
        return 0;

    node_c* curr, *prev = NULL;
    curr=(fifo_queue)->head;

    while (strcmp(curr->path, name_file) != 0)  {
        prev = curr;
        curr = curr->next;
    }

    prev->next = curr->next;
    free(curr);

    return 0;
}

//TODO: testa
int remove_fifo(){
    node_c *temp;

    if (fifo_queue->head == NULL)
        return 0;
    
    temp = fifo_queue->head;
    node_c *current = fifo_queue->head;
    fifo_queue->head = current->next;

    free(temp);
    
    return 0;
}