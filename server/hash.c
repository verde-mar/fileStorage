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

int add_hashtable(char *name_file, int flags){
    CHECK_OPERATION(name_file == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    node *exists = look_for_node(name_file);
    if(exists == NULL){
        /* Aggiunge l' elemento nella tabella hash */
        int success = 222;
        int hash = hash_function(name_file); //TODO:CREA
        CHECK_OPERATION(flags == 4, 
            int err_lock = set_lock(name_file);
                CHECK_OPERATION(err_lock==-1, 
                    fprintf(stderr, "Errore nel setting della lock del nodo.\n"); 
                        return -1);
                            return -1;);
        success = add(&(table->queue[hash]), name_file, flags);   
        CHECK_OPERATION(success==-1, 
            fprintf(stderr, "Errore nell'inserimento di un elemento nella tabella hash.\n"); 
                return -1);
        return success;
    }
    return 0;
}

int del_hashtable(char *name_file, node *just_deleted){
    int success = 222;
    int hash = 0;

    /* Se vuole eliminare un nodo preciso, ne calcola l'hash, altrimenti prende il primo */
    if(name_file!=NULL){
        /* Verifica se il nodo esiste */
        node *exists = look_for_node(name_file);

        /* Se il nodo esiste, ne calcola l'hash del nome */
        if(exists!=NULL)
            hash = hash_function(name_file); //TODO:CREA  
        /* Se il nodo non esiste, restituisce 0, perche' e' come se fosse stato eliminato */
        else
            return 0;
    }

    /* Elimina un nodo */
    success = delete(&(table->queue[hash]), name_file);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nell'eliminazione di un elemento nella tabella hash.\n"); 
            return -1);
    
    return success;
}

node* look_for_node(char* name_file){
    CHECK_OPERATION(name_file == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    int hash = hash_function(name_file); //TODO:CREA
    node* curr;
    for (curr=table->queue[hash]; curr != NULL; curr=curr->next)
        if (strcmp(curr->path, name_file) == 0)
            return(curr);

    return NULL;
}

int set_flag(char* name_file){
    CHECK_OPERATION(name_file != NULL,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    node* file = look_for_node(name_file);
    CHECK_OPERATION(file == NULL, 
        fprintf(stderr, "Il nodo cercato non esiste.\n");
            return -1;);
    CHECK_OPERATION(file->lock == 0,
        fprintf(stderr, "La lock non e' stata settata.\n");
            return -1);

    file->open = 1;
    
    return 0;
}

int set_lock(char* name_file){
    CHECK_OPERATION(name_file != NULL,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    node* file = look_for_node(name_file);
    CHECK_OPERATION(file == NULL, 
        fprintf(stderr, "Il nodo cercato non esiste.\n");
            return -1;);

    int success = set_mutex(file);
    CHECK_OPERATION(success == -1,
        fprintf(stderr, "Errore nel setting della mutex.\n");
            return -1;);
    return 0;
    
}

int unset_lock(char* name_file){
    CHECK_OPERATION(name_file != NULL,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    node* file = look_for_node(name_file);
    CHECK_OPERATION(file == NULL, 
        fprintf(stderr, "Il nodo cercato non esiste.\n");
            return -1;);
            
    int success = unset_mutex(file);
    CHECK_OPERATION(success == -1,
        fprintf(stderr, "Errore nel setting della mutex.\n");
            return -1;);
    return 0;
    
}