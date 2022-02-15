#include <stdio.h>
#include <hash.h>
#include <check_errors.h>

#include <stdlib.h>

int create_hashtable(size_t size){
    /* Inizializza la struttura dati della tabella */
    table = (hashtable*) malloc(sizeof(hashtable)*size);
    CHECK_OPERATION(table == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1;);

    /* Inizializza l'array delle liste di trabocco */
    table->queue = malloc(sizeof(list_t*)*16);

    /* Inizializza le liste di trabocco*/
    for (int i = 0; i < 16; i++) {
        int err = create_list(&(table->queue[i])); 
        CHECK_OPERATION(err == -1, return -1);
    }

    /* Inizia la coda FIFO */
    int create = create_fifo(&fifo_queue);
    CHECK_OPERATION(create == -1,
        fprintf(stderr, "Errore nella creazione della coda FIFO.\n");
            return -1);

    return 0;
}

int destroy_hashtable (){
    /* Elimina tutte le liste di trabocco */
    int destroy;
    for (int i = 0; i < 16; i++)
        if (table->queue[i]) {
            destroy = destroy_list(&(table->queue[i]));
            CHECK_OPERATION(destroy == -1, 
                fprintf(stderr, "Non e' stato possibile eliminare una lista di trabocco.\n");
                    return -1);
        }
    free(table->queue);

    /* Elimina la coda FIFO */
    int del = delete_fifo(&fifo_queue);
    CHECK_OPERATION(del == -1,
        fprintf(stderr, "Errore nella creazione della coda FIFO.\n");
            return -1);

    /* Elimina la tabella hash */
    free(table);

    return 0;
}

int add_hashtable(char *name_file, int fd_c, int flags){
    CHECK_OPERATION(name_file == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    /* Aggiunge l' elemento nella tabella hash */
    int success = -1;
    int hash = hash_function(name_file); //TODO:CREA
    success = add(&(table->queue[hash]), name_file, fd_c, flags);   
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nell'inserimento di un elemento nella tabella hash.\n"); 
            return -1);
    return success;
    
    
}

int del_hashtable(char *name_file, node *just_deleted, int fd){
    CHECK_OPERATION(name_file==NULL,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);
    int success = -1;
    int hash = 0;

    hash = hash_function(name_file); //TODO:CREA 
    /* Elimina un nodo */
    success = delete(&(table->queue[hash]), name_file, fd);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nell'eliminazione di un elemento nella tabella hash.\n"); 
            return -1);
    
    return success;
}

int close_hashtable(char *name_file, int fd){
    CHECK_OPERATION(name_file==NULL,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);
    int success = -1;
    int hash = 0;

    hash = hash_function(name_file); //TODO:CREA 
    /* Elimina un nodo */
    success = close(&(table->queue[hash]), name_file, fd);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella chiusura di un elemento nella tabella hash.\n"); 
            return -1);
}