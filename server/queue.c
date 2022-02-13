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

//TODO:testa
int add(list_t **lista_trabocco, char* name_file){
    CHECK_OPERATION(name_file == NULL || (*lista_trabocco) == NULL,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    node *curr = (node*)malloc(sizeof(node));
    CHECK_OPERATION(curr == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    curr->path = name_file;
    int m_init = pthread_mutex_init(&(curr->mutex), NULL);
    CHECK_OPERATION(m_init == -1,
        fprintf(stderr, "Errore nella inizializzazione della mutex.\n");
            return -1);
    curr->open = 0;
    curr->buffer = NULL;
    curr->next = (*lista_trabocco)->head; 
    (*lista_trabocco)->head = curr;

    return 0;
}

//TODO: testa
int delete(list_t **lista_trabocco, char* name_file, node **just_deleted){
    node* curr, *prev = NULL;
    curr=(*lista_trabocco)->head;

    while (strcmp(curr->path, name_file) != 0)  {
        prev = curr;
        curr = curr->next;
    }

    prev->next = curr->next;
    *just_deleted = curr; //TODO: eh ma poi la free?

    return 0;
}