#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

#include <check_errors.h>

int create_list(list_t **lista_trabocco){
    *lista_trabocco = malloc(sizeof(list_t));
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    (*lista_trabocco)->elements = 0;
    (*lista_trabocco)->head = NULL;
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

    free((*lista_trabocco)->head);
    int check_dest = pthread_mutex_destroy((*lista_trabocco)->mutex);
    CHECK_OPERATION(check_dest == -1,
        fprintf(stderr, "Non e' stato possibile distruggere la mutex della lista di trabocco.\n");
            return -1);
    free(*lista_trabocco);

    return 0;
}

int add(list_t **lista_trabocco, char* name_file){
    CHECK_OPERATION(name_file == NULL || !(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    node *curr = (node*)malloc(sizeof(node));
    CHECK_OPERATION(curr == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    curr->path = name_file;
    pthread_mutex_init(&(curr->mutex), NULL);
    curr->open = 0;
    curr->buffer = NULL;
    curr->next = (*lista_trabocco)->head; //va bene aggiungere cosi' in testa?

    (*lista_trabocco)->head = curr;
}

int delete(list_t **lista_trabocco, char* name_file, node **just_deleted){
    //TODO: restituisce 11 se l'elemento non esiste
    node* curr, *prev = NULL;

    for (curr=(*lista_trabocco)->head; curr != NULL; )  {
        
        prev = curr;
        curr = curr->next;
    }
}