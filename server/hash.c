#include <stdio.h>
#include <hash.h>
#include <check_errors.h>

hashtable* create_hashtable (int size){
    CHECK_OPERATION(size<0, 
        fprintf(stderr, "Size passata come parametro non valida.\n");
            return -1); 
    table->size = size;

    /* Alloca table */
    table = (hashtable*) malloc(sizeof(hashtable));
    CHECK_OPERATION(table==NULL, 
        fprintf(stderr, " errore nella lettura della size del messaggio.\n");
            return -1); 

    /* Alloca table->files */
    table->files = malloc(sizeof(hashtable)*size);
    CHECK_OPERATION(table->files == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);
    for (int i = 0; i < table->size; i++) {
        table->files[i] = create_list();
    }

    /* Alloca table->queue */
    table->queue = malloc(sizeof(hashtable)*size);
        CHECK_OPERATION(table->queue == NULL,
            fprintf(stderr, "Allocazione non andata a buon fine.\n");
                return -1);
    for (int i = 0; i < table->size; i++) {
        table->queue[i] = create_list();
    }
}