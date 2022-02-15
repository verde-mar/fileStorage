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

    return 0;
}

int destroy_list(list_t **lista_trabocco){
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    node *tmp = NULL;
    while ((*lista_trabocco)->head) {
        tmp = (*lista_trabocco)->head;
        (*lista_trabocco)->head = ((*lista_trabocco)->head)->next;
        free((char*)tmp->path);
        if(tmp->buffer != NULL) free(tmp->buffer);
        free(tmp);
    }
    
    int check_dest = pthread_mutex_destroy((*lista_trabocco)->mutex);
    CHECK_OPERATION(check_dest == -1,
        fprintf(stderr, "Non e' stato possibile distruggere la mutex della lista di trabocco.\n");
            return -1);
    free(*lista_trabocco);

    return 0;
}

int add(list_t **lista_trabocco, char* name_file, int fd, int flags){
    CHECK_OPERATION(name_file == NULL || (*lista_trabocco) == NULL,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    if(flags == 2 || flags == 6){
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

        /* Aggiunge il nodo in testa alla lista di trabocco, se non esiste gia' */
        int check_exists = look_for_node(*lista_trabocco, name_file);
        CHECK_OPERATION(check_exists == 1,
            fprintf(stderr, "Il nodo esiste gia'.\n");
                return 101);

        curr->next = (*lista_trabocco)->head; 
        (*lista_trabocco)->head = curr;
        curr->fd_c = fd;

        pthread_mutex_unlock((*lista_trabocco)->mutex);
    }

    if(flags == 6 || flags == 4){
        /* Setta la mutex */
        int err_set = set_mutex(name_file, fd);
        CHECK_OPERATION(err_set==-1,
            fprintf(stderr, "Errore nel setaggio della mutex");
                return -1);
    }

    CHECK_OPERATION((flags!=2 || flags!=4) || flags !=6, 
        fprintf(stderr, "Flag non validi.\n");
            return 404;);

    return 0;
}

node* delete(list_t **lista_trabocco, char* name_file, int fd){
    CHECK_OPERATION(!*lista_trabocco || !name_file || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    pthread_mutex_lock((*lista_trabocco)->mutex);

    node* curr, *prev;
    curr = (*lista_trabocco)->head;
    if (strcmp(curr->path, name_file) == 0){
        /* Se il flag di apertura e' pari ad 1 
                e il client ne aveva gia' acquisito la lock 
                    si procede con l'eliminazione */
        if(curr->open == 1 && curr->fd_c == fd){
            (*lista_trabocco)->head = curr->next; 
            pthread_mutex_unlock((*lista_trabocco)->mutex);
            
            return curr;
        } 
        /* Altrimenti viene restituito NULL */
        else {
            pthread_mutex_unlock((*lista_trabocco)->mutex);
            
            return NULL;
        }
    }

    prev = curr;
    curr = curr->next;
    while (curr != NULL) {
        if (strcmp(curr->path, name_file) == 0){
            /* Se il flag di apertura e' pari ad 1 
                e il client ne aveva gia' acquisito la lock 
                    si procede con l'eliminazione */
            if(curr->open == 1 && curr->fd_c == fd){
                prev->next = curr->next; 
                pthread_mutex_unlock((*lista_trabocco)->mutex);
                return curr;
            } 
            /* Altrimenti viene restituito NULL */
            else {
                pthread_mutex_unlock((*lista_trabocco)->mutex);
                return NULL;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    /* Il nodo non e' stato trovato */
    pthread_mutex_unlock((*lista_trabocco)->mutex);

    return NULL;
}

int look_for_node(list_t **lista_trabocco, char* name_file){
    node* curr;
    for (curr=(*lista_trabocco)->head; curr != NULL; curr=curr->next)
        if (strcmp(curr->path, name_file) == 0)
            return(curr);
}

int close(list_t **lista_trabocco, char* name_file, int fd){
    CHECK_OPERATION(!*lista_trabocco || !name_file || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    node* nodo = look_for_node(*lista_trabocco, name_file);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
            return -1);

    pthread_mutex_lock(nodo->mutex);
    if(nodo->fd_c == fd && nodo->open == 1){
        nodo->open = 0;
        nodo->fd_c = -1;
    } else if(nodo->fd_c != fd){
        pthread_mutex_unlock(nodo->mutex);
        return 202;
    } else if(nodo->open == 0){
        pthread_mutex_unlock(nodo->mutex);
        return 303;
    }
    
    pthread_mutex_unlock(nodo->mutex);
    return 0;
}
