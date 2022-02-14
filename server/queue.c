#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

#include <check_errors.h>
#include <string.h>

int create_list(list_t **lista_trabocco){
    *lista_trabocco = malloc(sizeof(list_t));
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    (*lista_trabocco)->elements = 0;
    (*lista_trabocco)->head = NULL;
    int mutex_init = pthread_mutex_init((*lista_trabocco)->mutex, NULL);
    CHECK_OPERATION(mutex_init == -1,
        fprintf(stderr, "Non e' stato possibile inizializzare la mutex della lista di trabocco.\n");
            return -1);
    int cond_init = pthread_cond_init((*lista_trabocco)->empty, NULL);
    CHECK_OPERATION(cond_init == -1,
        fprintf(stderr, "Non e' stato possibile inizializzare la variabile di condizione della lista di trabocco.\n");
            return -1);

    return 0;
}

int destroy_list(list_t **lista_trabocco){
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    if(*lista_trabocco == NULL)
        return 0;
    else {
        node *tmp = NULL;
        while ((*lista_trabocco)->head) {
            tmp = (*lista_trabocco)->head;
            (*lista_trabocco)->head = ((*lista_trabocco)->head)->next;
            free((char*)tmp->path);
            free(tmp);
        }
    }
    
    int check_dest = pthread_mutex_destroy((*lista_trabocco)->mutex);
    CHECK_OPERATION(check_dest == -1,
        fprintf(stderr, "Non e' stato possibile distruggere la mutex della lista di trabocco.\n");
            return -1);
    free(*lista_trabocco);

    return 0;
}

int add(list_t **lista_trabocco, char* name_file){
    CHECK_OPERATION(name_file == NULL || (*lista_trabocco) == NULL,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Crea il nodo da aggiungere */
    node *curr = (node*)malloc(sizeof(node));
    CHECK_OPERATION(curr == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);
    curr->path = malloc(sizeof(char)*(strlen(name_file)+1));
    strcpy((char*)curr->path, name_file);
    int m_init = pthread_mutex_init(curr->mutex, NULL);
    CHECK_OPERATION(m_init == -1,
        fprintf(stderr, "Errore nella inizializzazione della mutex.\n");
            return -1);
    curr->open = 0;
    curr->buffer = NULL;

    pthread_mutex_lock((*lista_trabocco)->mutex);

    /* Aggiunge il nodo in testa alla lista di trabocco*/
    curr->next = (*lista_trabocco)->head; 
    (*lista_trabocco)->head = curr;

    pthread_cond_signal((*lista_trabocco)->empty);

    pthread_mutex_unlock((*lista_trabocco)->mutex);

    return 0;
}

node* delete(list_t **lista_trabocco, char* name_file){
    CHECK_OPERATION(!*lista_trabocco || !name_file,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    
    pthread_mutex_lock((*lista_trabocco)->mutex);

    while((*lista_trabocco)->elements == 0)
        pthread_cond_wait((*lista_trabocco)->empty, (*lista_trabocco)->mutex);

    node* curr, *prev;
    curr = (*lista_trabocco)->head;
    if (strcmp(curr->path, name_file) == 0){
        (*lista_trabocco)->head = curr->next; 
        pthread_mutex_unlock((*lista_trabocco)->mutex);
        return curr;
    }

    prev = curr;
    curr = curr->next;
    while (curr != NULL) {
        if (strcmp(curr->path, name_file) == 0){
            prev->next = curr->next; 
            pthread_mutex_unlock((*lista_trabocco)->mutex);
            return curr;
        }
        prev = curr;
        curr = curr->next;
    }
    pthread_mutex_unlock((*lista_trabocco)->mutex);

    return NULL;
}

