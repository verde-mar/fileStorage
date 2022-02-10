#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

#include <check_errors.h>

int create_list(list_t **lista_trabocco){
    *lista_trabocco = malloc(sizeof(struct list));
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    (*lista_trabocco)->elements = 0;
    (*lista_trabocco)->tail = NULL;
    int check_init = pthread_mutex_init((*lista_trabocco)->mutex, NULL);
    CHECK_OPERATION(check_init == -1,
        fprintf(stderr, "Non e' stato possibile inizializzare la mutex della lista di trabocco.\n");
            return -1);

    return 0;
}


int destroy_list(list_t **lista_trabocco){
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    free((*lista_trabocco)->tail);
    int check_dest = pthread_mutex_destroy((*lista_trabocco)->mutex);
    CHECK_OPERATION(check_dest == -1,
        fprintf(stderr, "Non e' stato possibile distruggere la mutex della lista di trabocco.\n");
            return -1);
    free(*lista_trabocco);

    return 0;
}