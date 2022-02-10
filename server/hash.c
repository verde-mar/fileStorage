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
    table->num_file = 0;

    /* Inizializza l'array delle liste di trabocco */
    table->queue = malloc(sizeof(list_t*)*16);

    /* Inizializza le liste di trabocco*/
    for (int i = 0; i < 16; i++) {
        int err = create_list(&(table->queue[i])); 
        CHECK_OPERATION(err == -1, return -1);
    }

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

    /* Elimina la tabella hash */
    free(table);

    return 0;
}

int add_list(char *name_file){
    CHECK_OPERATION(name_file == NULL, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    int success = 222;
    int hash = hash_function(name_file); //TODO:CREA
    success = insert_list(&(table->queue[hash]), name_file);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nell'inserimento di un elemento nella tabella hash.\n"); 
            return -1);

    /* Incrementa il numero di elementi nella tabella */
    if (success == 0) table->num_file++;

    return success;
}