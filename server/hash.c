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
        table->queue[i] = create_list(); 
    }

    return 0;
}

int destroy_hashtable (){
    /* Elimina tutte le liste di trabocco */
    int destroy;
    for (int i = 0; i < 16; i++)
        if (table->queue[i]) {
            destroy = destroy_list(table->queue[i]);
            CHECK_OPERATION(destroy == -1, 
                fprintf(stderr, "Non e' stato possibile eliminare una lista di trabocco.\n");
                    return -1);
        }
    free(table->queue);

    /* Elimina la tabella hash */
    free(table);

    return 0;
}