#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

#include <check_errors.h>
#include <string.h>
#include <errno.h>

#include <fifo.h>

int create_list(list_t **lista_trabocco){
    /* Inizializza la lista */
    *lista_trabocco = malloc(sizeof(list_t));
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    /* Inizializza il numero di elementi */
    (*lista_trabocco)->elements = 0;
    /* Inizializza la testa */
    (*lista_trabocco)->head = NULL;
    /* Inizializza la mutex */
    (*lista_trabocco)->mutex = malloc(sizeof(pthread_mutex_t));
    PTHREAD_INIT_LOCK((*lista_trabocco)->mutex);

    return 0;
}

int destroy_list(list_t **lista_trabocco){
    /* Se la lista di trabocco e' uguale a NULL non e' mai stata allocata */
    CHECK_OPERATION(*lista_trabocco == NULL,
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    node *tmp = NULL;
    /* Libera la memoria occupata da ciascun nodo */
    while ((*lista_trabocco)->head) {
        tmp = (*lista_trabocco)->head;
        (*lista_trabocco)->head = ((*lista_trabocco)->head)->next;

        /* Libera la memoria del nodo corrente */
        PTHREAD_DESTROY_LOCK(tmp->mutex);
        PTHREAD_DESTROY_COND(tmp->locked); 
        free(tmp->mutex);
        free(tmp->locked);

        free((char*)tmp->path);

        if(tmp->buffer != NULL) free(tmp->buffer);

        free(tmp);
    }
    /* Distrugge la lock della lista di trabocco */
    PTHREAD_DESTROY_LOCK((*lista_trabocco)->mutex)
    free((*lista_trabocco)->mutex);

    /* Libera la memoria occupata dalla lista di trabocco */
    free(*lista_trabocco);

    return 0;
}

int add(list_t **lista_trabocco, char* name_file, int fd, int flags){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Aggiunge il nodo in testa alla lista di trabocco, se non esiste gia' */
    node* check_exists = look_for_node(lista_trabocco, name_file);
    CHECK_OPERATION(check_exists != NULL,
        fprintf(stderr, "Il nodo esiste gia'.\n");
                    return 101);
   
    /* Se e' stata specificata l'operazione di creazione */
    if(flags == 2 || flags == 6){
        /* Crea il nodo da aggiungere */
        node *curr = (node*)malloc(sizeof(node));
        CHECK_OPERATION(curr == NULL,
            fprintf(stderr, "Allocazione non andata a buon fine.\n");
                return -1);

        curr->path = malloc(sizeof(char)*(strlen(name_file)+1));
        CHECK_OPERATION(curr->path == NULL,
            fprintf(stderr, "Allocazione non andata a buon fine.\n");
                return -1);
        
        strcpy((char*)curr->path, name_file);
        curr->open = 1;
        curr->buffer = NULL;
        curr->fd_c = fd;
        curr->locked = malloc(sizeof(pthread_cond_t));
        PTHREAD_INIT_COND(curr->locked);
        curr->mutex = malloc(sizeof(pthread_mutex_t));
        PTHREAD_INIT_LOCK(curr->mutex);

        PTHREAD_LOCK(fifo_queue->mutex);
        PTHREAD_LOCK((*lista_trabocco)->mutex);
        
        int adder = add_fifo(name_file);
        CHECK_OPERATION(adder == -1, return -1);

        curr->next = (*lista_trabocco)->head; 
        (*lista_trabocco)->head = curr;
        
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);
    }

    /* Se e' stata specificata l'operazione di acquisizione della lock */
    if(flags == 6 || flags == 4){

        /* Se il nodo esiste acquisisce la lock */
        int check_lock = lock(lista_trabocco, name_file, fd);
        CHECK_OPERATION(check_lock == -1,
            fprintf(stderr, "Il nodo non e' stato trovato.\n");
                        return 505);

    }

    /* Se non e' stata specificata nessuna operazione valida, allora restituisce un errore */
    CHECK_OPERATION((flags!=2 && flags!=4) && flags !=6, 
        fprintf(stderr, "Flag non validi.\n");
            return 404;);
    
    return 0;
}

//TODO: puoi abbreviarla usando la look_for_node
int delete(list_t **lista_trabocco, char* name_file, node** just_deleted, int fd){
    CHECK_OPERATION(*lista_trabocco==NULL,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    PTHREAD_LOCK(fifo_queue->mutex);

    PTHREAD_LOCK((*lista_trabocco)->mutex);

    int remover = del(name_file);
    CHECK_OPERATION(remover == -1, return -1);

    node* curr, *prev;
    prev = NULL;
    curr = (*lista_trabocco)->head;

    while (curr != NULL) {
        if (strcmp(curr->path, name_file) == 0){
            /* Se il flag di apertura e' pari ad 1 
                e il client ne aveva gia' acquisito la lock 
                    si procede con l'eliminazione */
            if(curr->open == 1 && curr->fd_c == fd){
                prev->next = curr->next; 
                *just_deleted = curr;
                int success = del(name_file);
                CHECK_OPERATION(success==-1, 
                    fprintf(stderr, "Errore nell'inserimento di un elemento nella coda fifo.\n"); 
                        return -1);
                PTHREAD_UNLOCK((*lista_trabocco)->mutex);
                PTHREAD_UNLOCK(fifo_queue->mutex);
                
                return 0;
            } 
            /* Il file non e' stato aperto */
            else if(curr->open == 0){
                PTHREAD_UNLOCK((*lista_trabocco)->mutex);
                PTHREAD_UNLOCK(fifo_queue->mutex);
                
                return 303;
                
            } 
            /* Non e' stata acquisita la lock */
            else if(curr->fd_c != fd){
                PTHREAD_UNLOCK((*lista_trabocco)->mutex);
                PTHREAD_UNLOCK(fifo_queue->mutex);

                return 202;
            }
        }
        prev = curr;
        curr = curr->next;
    }

    /* Il nodo non e' stato trovato */
    *just_deleted = NULL;
    PTHREAD_UNLOCK((*lista_trabocco)->mutex);
    PTHREAD_UNLOCK(fifo_queue->mutex);

    return -1;
}

node* look_for_node(list_t **lista_trabocco, char* name_file){
    node* curr;
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    for (curr=(*lista_trabocco)->head; curr != NULL; curr=curr->next)
        if (strcmp(curr->path, name_file) == 0){
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);
            return(curr);
        }
    PTHREAD_UNLOCK((*lista_trabocco)->mutex);
    return NULL;
}

int close(list_t **lista_trabocco, char* name_file, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, name_file);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
                    return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se ha acquisito la lock e il flag open e' ad 1 */
    if(nodo->fd_c == fd && nodo->open == 1){
        nodo->open = 0;
        PTHREAD_UNLOCK(nodo->mutex);
        return 0;
    } 
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 202;
    } 
    /* Il file non e' stato aperto */
    else if(nodo->open == 0){
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    
    PTHREAD_UNLOCK(nodo->mutex);

    return -1;
}

int unlock(list_t **lista_trabocco, char* name_file, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, name_file);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
                    return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se ha acquisito la lock e il flag open e' a 1 */
    if(nodo->fd_c == fd && nodo->open == 1){
        nodo->fd_c = -1;
        PTHREAD_COND_SIGNAL(nodo->locked);
    } 
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 202;
    } 
    /* Il file non e' stato aperto */
    else if(nodo->open == 0){
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

int lock(list_t **lista_trabocco, char* name_file, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, name_file);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
                    return 505);
    
    PTHREAD_LOCK(nodo->mutex);
    /* Se la lock era gia' stata acquisita dallo stesso processo o era libera */
    if(nodo->fd_c == fd || nodo->fd_c == -1){
        while(nodo->fd_c != -1 && nodo->fd_c != fd)
            PTHREAD_COND_WAIT(nodo->mutex, nodo->locked);
        
        nodo->fd_c = fd;
    } else {
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

int reads(list_t **lista_trabocco, char* name_file, char** buf, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, name_file);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
                    return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se ha acquisito la lock e il flag open e' a 1 */
    if(nodo->fd_c == fd && nodo->open == 1){
        *buf = malloc(sizeof(char)*strlen(nodo->buffer)); 
        strcpy(*buf, nodo->buffer);
        PTHREAD_UNLOCK(nodo->mutex);

        return 0;
    } 
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        PTHREAD_UNLOCK(nodo->mutex);
        return 202;
    } 
    /* Il file non e' stato aperto */
    else if(nodo->open == 0){
        PTHREAD_UNLOCK(nodo->mutex);
        return 303;
    }
    
    PTHREAD_UNLOCK(nodo->mutex);
    
    return 0;
}

int append_buffer(list_t **lista_trabocco, char* name_file, char* buf, int size_buf, int* max_size, int* curr_size, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, name_file);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
                    return 505);

    PTHREAD_LOCK(fifo_queue->mutex);

    if(size_buf<=*max_size){
        *curr_size += size_buf;
        if(*curr_size > *max_size){
            char* to_delete = remove_fifo();
            if(to_delete){
                int delete = del(name_file);
                CHECK_OPERATION(delete == -1, PTHREAD_UNLOCK(fifo_queue->mutex); return -1);
                CHECK_OPERATION(delete != -1, PTHREAD_UNLOCK(fifo_queue->mutex); return 909);
            }
        }
    }
    PTHREAD_LOCK(nodo->mutex);

    /* Se ha acquisito la lock e il flag open e' a 1 */
    if(nodo->fd_c == fd && nodo->open == 1){
        int len = (strlen(nodo->buffer) + size_buf) + 1; 
        nodo->buffer = (char*) realloc(nodo->buffer, len);
        nodo->buffer = strcat(nodo->buffer, buf);
        PTHREAD_UNLOCK(nodo->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);

        return 0;
    } 
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        PTHREAD_UNLOCK(nodo->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);

        return 202;
    } 
    /* Il file non e' stato aperto */
    else if(nodo->open == 0){
        PTHREAD_UNLOCK(nodo->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);

        return 303;
    }
    
    PTHREAD_UNLOCK(nodo->mutex);
    PTHREAD_UNLOCK(fifo_queue->mutex);

    return -1;
}

int writes(list_t **lista_trabocco, char* name_file, char* buf, int size_buf, int *max_size, int* curr_size, int fd){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, name_file);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
                    return 505);

    PTHREAD_LOCK(fifo_queue->mutex);
    
    if(size_buf<=*max_size){
        *curr_size += size_buf;
        if(*curr_size > *max_size){
            char* to_delete = remove_fifo();
            if(to_delete){
                int delete = del(name_file);
                CHECK_OPERATION(delete == -1, PTHREAD_UNLOCK(fifo_queue->mutex); return -1);
                CHECK_OPERATION(delete != -1, PTHREAD_UNLOCK(fifo_queue->mutex); return 909);
            } else {
                PTHREAD_UNLOCK(fifo_queue->mutex); 
                return -1;
            }
        }
    }
    
    PTHREAD_LOCK(nodo->mutex);
    printf("CAGNOTTO\n");
    /* Se ha acquisito la lock e il flag open e' a 1 */
    if(nodo->fd_c == fd && nodo->open == 1){
        nodo->buffer =  malloc(sizeof(char)*(size_buf+1));
        nodo->buffer = strcpy(nodo->buffer, buf);
        PTHREAD_UNLOCK(nodo->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);

        return 0;
    } 
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        PTHREAD_UNLOCK(nodo->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);
        return 202;
    } 
    /* Il file non e' stato aperto */
    else if(nodo->open == 0){
        PTHREAD_UNLOCK(nodo->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);
        return 303;
    }
    
    PTHREAD_UNLOCK(nodo->mutex);
    PTHREAD_UNLOCK(fifo_queue->mutex);

    return 0;
}

