#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

#include <check_errors.h>
#include <string.h>
#include <errno.h>

#include <fifo.h>
#include <utils.h>

int create_list(list_t **lista_trabocco){
    /* Crea la lista */
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
    CHECK_OPERATION((*lista_trabocco)->mutex == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
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
        PTHREAD_DESTROY_LOCK(tmp->mutex, "destroy_list: tmp->mutex");
        PTHREAD_DESTROY_COND(tmp->locked); 
        free(tmp->mutex);
        free(tmp->locked);
        if(tmp->buffer)
            free(tmp->buffer);
        free((char*)tmp->path);

        free(tmp);
    }
    /* Distrugge la lock della lista di trabocco */
    PTHREAD_DESTROY_LOCK((*lista_trabocco)->mutex, "destroy_list: lista_trabocco->mutex");
    free((*lista_trabocco)->mutex);

    /* Libera la memoria occupata dalla lista di trabocco */
    free(*lista_trabocco);

    return 0;
}

/**
 * @brief Effettua la creazione effettiva di un nodo
 * 
 * @param queue Lista di trabocco in cui aggiungere il nodo
 * @param file_path Path del file da associare al nodo da creare
 * @param fd file descriptor di chi ha creato e acquisito la lock sul nodo
 * @return node* Nodo creato
 */
static node* node_create(list_t **queue, char* file_path, int fd, int flags){
    CHECK_OPERATION(!(*queue),
        fprintf(stderr, "Parametri non validi.\n");
        return NULL);

    /* Crea il nodo */
    node *curr = (node*)malloc(sizeof(node));
    CHECK_OPERATION(curr == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);

    /* Inizializza le maschere */
    FD_ZERO(&(curr->open));
    FD_ZERO(&(curr->operation_open));

    curr->path = malloc(sizeof(char)*(strlen(file_path)+1));
    CHECK_OPERATION(curr->path == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    
    strcpy((char*)curr->path, file_path);
    FD_SET(fd, &(curr->open));
    FD_SET(fd, &(curr->operation_open));

    curr->size_buffer = 0;
    curr->buffer = NULL;
    curr->written = 0;
    if(flags!=2)
        curr->fd_c = fd;
    else
        curr->fd_c = -1;

    curr->locked = malloc(sizeof(pthread_cond_t));
    CHECK_OPERATION(curr->locked == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    PTHREAD_INIT_COND(curr->locked);
    curr->mutex = malloc(sizeof(pthread_mutex_t));
    CHECK_OPERATION(curr->mutex == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    PTHREAD_INIT_LOCK(curr->mutex);

    return curr;
}

int add(list_t **lista_trabocco, char* file_path, int fd, int flags, int *max_file_reached, FILE* file_log){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    /* Aggiunge il nodo in testa alla lista di trabocco, se non esiste gia' */
    node* nodo = look_for_node(lista_trabocco, file_path);
    
    /* Se e' stata specificata l'operazione di creazione */
    if((flags == 2 || flags == 6) && nodo == NULL){
        nodo = node_create(lista_trabocco, file_path, fd, flags);
        CHECK_OPERATION(nodo == NULL, return -1);

        PTHREAD_LOCK(fifo_queue->mutex);
        PTHREAD_LOCK((*lista_trabocco)->mutex);
       
        int adder = add_fifo((char*)nodo->path);
        CHECK_OPERATION(adder == -1, 
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);
            return -1);
        *max_file_reached = max(*max_file_reached, fifo_queue->elements);
        nodo->next = (*lista_trabocco)->head; 
        (*lista_trabocco)->head = nodo;
        
        fprintf(file_log, "E' stato creato il nodo con path: %s\n", nodo->path);

        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        return 0;
    } else if((flags == 5 || flags == 0 || flags == 4) && nodo != NULL){
        PTHREAD_LOCK(nodo->mutex);
        FD_SET(fd, &(nodo->open));
        FD_SET(fd, &(nodo)->operation_open);
        fprintf(file_log, "E' stato aperto il nodo con path: %s\n", nodo->path);
        
        PTHREAD_UNLOCK(nodo->mutex);
        if(!flags) return 0;
    }

    if(flags == 4 || flags == 5){
        PTHREAD_LOCK(nodo->mutex);
        fprintf(file_log, "E' stata acquisita la lock sul nodo con path: %s\n", nodo->path);
        nodo->fd_c = fd;
        PTHREAD_UNLOCK(nodo->mutex);
        return 0;
    } 

    /* Se non e' stata specificata nessuna operazione valida, allora restituisce un errore */
    CHECK_OPERATION((flags!=2 && flags !=6) && (flags!=4 && flags != 0) && flags != 5, 
        fprintf(stderr, "Flag non validi.\n");
        return 404;);

    CHECK_OPERATION((flags == 2 || flags == 6) && nodo!=NULL, fprintf(stderr, "Il nodo %s esiste gia'.\n", nodo->path); return 101);
    CHECK_OPERATION(nodo == NULL, fprintf(stderr, "Il nodo non e' stato trovato.\n"); return 505);

    return 505;
}

int deletes(list_t **lista_trabocco, char* file_path, node** just_deleted, int fd, int *curr_size, FILE* file_log){
    CHECK_OPERATION(*lista_trabocco==NULL,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato.\n");
        return 505);
            
    PTHREAD_LOCK(fifo_queue->mutex);
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    int remover = del(file_path);
    CHECK_OPERATION(remover == -1, 
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        return -1);
    
    node* curr; /* Puntatore al nodo corrente */
    if ((*lista_trabocco)->head == NULL){ /* Lista vuota */
        fprintf(file_log, "Si e' tentato di eliminare un nodo, ma la lista era vuota.\n");
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);

        return 0;
    }
    curr = (*lista_trabocco)->head;
    if (strcmp(curr->path, file_path) == 0) { /* Cancellazione del primo nodo */
        if(curr->fd_c == fd && FD_ISSET(fd, &(curr->open))){
            *just_deleted = curr;
            (*lista_trabocco)->head = curr->next; /* Aggiorna il puntatore alla testa */
            *curr_size = *curr_size - curr->size_buffer;

            FD_CLR(fd, &(curr->operation_open));
            FD_CLR(fd, &(curr->open));
            fprintf(file_log, "E' stato eliminato il nodo con path: %s\n", curr->path);

            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 0;
        } 
        /* Il file non e' aperto */
        else if(!FD_ISSET(fd, &(curr->open))){
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 303;
        }
        /* Non e' stata acquisita la lock */
        else if(curr->fd_c!=fd){
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 202;
        }
        
    }

    /* Scansione della lista a partire dal secondo nodo */
    node* prev = curr;
    curr = curr->next;
    while (curr != NULL) {
        if(curr->fd_c == fd && FD_ISSET(fd, &(curr->open))){
            *just_deleted = curr;
            prev->next = curr->next; /* Aggiorna il puntatore alla testa */
            *curr_size = *curr_size - curr->size_buffer;

            FD_CLR(fd, &(curr->operation_open));
            FD_CLR(fd, &(curr->open));

            fprintf(file_log, "E' stato creato il nodo con path: %s\n", curr->path);

            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 0;
        } 
        /* Il file non e' aperto */
        else if(!FD_ISSET(fd, &(curr->open))){
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 303;
        }
        /* Non e' stata acquisita la lock */
        else if(curr->fd_c!=fd){
            PTHREAD_UNLOCK(fifo_queue->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);

            return 202;
        }
        prev = curr;
        curr = curr->next;
    }

    PTHREAD_UNLOCK(fifo_queue->mutex);
    PTHREAD_UNLOCK((*lista_trabocco)->mutex);

    return -1;
}

node* look_for_node(list_t **lista_trabocco, char* file_path){
    node* curr;
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    for (curr=(*lista_trabocco)->head; curr != NULL; curr=curr->next)
        if (strcmp(curr->path, file_path) == 0){
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);
            return curr;
        }
    PTHREAD_UNLOCK((*lista_trabocco)->mutex);
    return NULL;
}

int closes(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL, fprintf(stderr, "Il nodo %s non e' stato trovato.\n", file_path); return 505);

    PTHREAD_LOCK(nodo->mutex);
    
    /* Se il nodo e' aperto */
    if(FD_ISSET(fd, &(nodo->open))){
        FD_CLR(fd, &(nodo->open));
        FD_CLR(fd, &(nodo->operation_open));

        fprintf(file_log, "E' stato chiuso il nodo con path: %s\n", nodo->path);

        PTHREAD_UNLOCK(nodo->mutex);
        
        return 0;
    } 
    /* Il nodo non e' stato aperto */
    else {
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    
    PTHREAD_UNLOCK(nodo->mutex);

    return -1;
}

int unlock(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo %s non e' stato trovato.\n", file_path);
        return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se ha acquisito la lock e il nodo e' aperto */
    if(nodo->fd_c == fd && FD_ISSET(fd, &(nodo->open))){
        nodo->fd_c = -1;
        FD_CLR(fd, &(nodo->operation_open));
        fprintf(file_log, "E' stata rilasciata la lock sul nodo con path: %s\n", nodo->path);
    } 
    /* Se il nodo non e' aperto */
    else if(!FD_ISSET(fd, &(nodo->open))){
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 303;
    }
    /* Se la lock non e' stata acquisita da nessuno */
    else if(nodo->fd_c == -1){
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 555;
    }
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        PTHREAD_UNLOCK(nodo->mutex);
        
        return 202;
    } 
    
    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

int lock(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);
    
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL, fprintf(stderr, "Il nodo non e' stato trovato. LOCK\n"); return 505);
                    
    PTHREAD_LOCK(nodo->mutex);

    /* Se la lock era gia' stata acquisita dallo stesso processo o era libera e il nodo e' aperto */
    if(FD_ISSET(fd, &(nodo->open)) && (nodo->fd_c == fd || nodo->fd_c == -1)){     
        nodo->fd_c = fd;
        FD_CLR(fd, &(nodo->operation_open));
        fprintf(file_log, "E' stata acquisita la lock sul nodo con path: %s\n", nodo->path);
    } 
    /* Se il nodo non e' aperto */
    else if(!FD_ISSET(fd, &(nodo->open))){
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    /* Se la lock e' stata acquisita da un altro thread */
    else {
        PTHREAD_UNLOCK(nodo->mutex);

        return 202;
    }

    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

int reads(list_t **lista_trabocco, char* file_path, void** buf, size_t *size_buf, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        *buf = NULL;
        *size_buf = 0;
        fprintf(stderr, "Il nodo non e' stato trovato. READS\n"); return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se il nodo e' aperto*/
    if(FD_ISSET(fd, &(nodo->open)) && nodo->fd_c == fd){
        *size_buf = nodo->size_buffer;
        *buf = nodo->buffer;
        FD_CLR(fd, &(nodo->operation_open));
        fprintf(file_log, "E' stata letto il buffer del nodo con path: %s\n", nodo->path);
    } 
    /* Se la lock e' stata acquisita da un altro thread */
    else if(nodo->fd_c!=fd && nodo->fd_c!=-1){
        PTHREAD_UNLOCK(nodo->mutex);

        return 202;
    }
    /* Se non e' stata acquisita la lock */
    else if(nodo->fd_c==-1){
        PTHREAD_UNLOCK(nodo->mutex);

        return 555;
    }
    /* Se il nodo e' chiuso */
    else {
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    PTHREAD_UNLOCK(nodo->mutex);
    
    return 0;
}

int append_buffer(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int* max_size, int* curr_size, int* max_size_reached, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
            return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato. APPEND_BUFFER\n");
        return 505);

    PTHREAD_LOCK(nodo->mutex);

    /* Se ha acquisito la lock e il nodo e' aperto */
    if(nodo->fd_c == fd && FD_ISSET(fd, &(nodo->open))){
        nodo->buffer = realloc(nodo->buffer, nodo->size_buffer + size_buf); 
        CHECK_OPERATION(nodo->buffer == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
            PTHREAD_UNLOCK(nodo->mutex);
            free(buf);
            return -1);
            
        memcpy(nodo->buffer + nodo->size_buffer, buf, size_buf);  
        nodo->size_buffer += (size_buf); 
        free(buf);
        FD_CLR(fd, &(nodo->operation_open));
        *max_size_reached = max(*curr_size, *max_size_reached);
        fprintf(file_log, "E' stata fatta la append sul nodo con path: %s di %ld byte.\n", nodo->path, size_buf);

        PTHREAD_UNLOCK(nodo->mutex);
        
        return 0;
    } 
    /* Se non ha acquisito la lock */
    else if(nodo->fd_c != fd){
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 202;
    } 
    /* Il nodo non e' stato aperto */
    else if(!FD_ISSET(fd, &(nodo->open))){
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    if(buf) free(buf);
    PTHREAD_UNLOCK(nodo->mutex);

    return -1;
}

int writes(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int *max_size, int* curr_size, int* max_size_reached, node** deleted, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);
            
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        fprintf(stderr, "Il nodo non e' stato trovato. WRITES\n");
        size_buf = 0;
        free(buf);
        return 505);
    
    PTHREAD_LOCK(nodo->mutex);
    if(FD_ISSET(fd, &(nodo->operation_open))){
        /* Se ha acquisito la lock e il flag open e' a 1 */
        if(nodo->fd_c == fd && (FD_ISSET(fd, &(nodo->open)) && buf) && !nodo->written){
            nodo->buffer = malloc(size_buf);
            CHECK_OPERATION(nodo->buffer == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return -1);
            memcpy(nodo->buffer, buf, size_buf);  
            nodo->size_buffer = size_buf;
            free(buf);
            FD_CLR(fd, &(nodo->operation_open));
            nodo->written = 1;
            *max_size_reached = max(*curr_size, *max_size_reached);
            fprintf(file_log, "E' stata fatta la write sul nodo con path: %s di %ld byte.\n", nodo->path, size_buf);

            PTHREAD_UNLOCK(nodo->mutex);
        
            return 0;
        } 
        else if(nodo->fd_c == fd && FD_ISSET(fd, &(nodo->open)) && !nodo->written){
            nodo->written = 1;
            PTHREAD_UNLOCK(nodo->mutex);
        
            return 0;
        }
        /* Se non ha acquisito la lock */
        else if(nodo->fd_c != fd){
            if(buf) free(buf);
            PTHREAD_UNLOCK(nodo->mutex);
            return 202;
        } 
        /* Se il file non e' stato aperto */
        else if(!FD_ISSET(fd, &(nodo->open))){
            if(buf) free(buf);
            PTHREAD_UNLOCK(nodo->mutex);

            return 303;
        } 
        /* Se e' gia' stata fatta la writeFile */
        else if(nodo->written){
            if(buf) free(buf);
            PTHREAD_UNLOCK(nodo->mutex);
            return 808;
        }
        
    } 
    /* Se non e' stata fatta la open */
    else {
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 606;
    }

    if(buf) free(buf);
    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

void print_hash(list_t *queue){
    node* curr = queue->head;
    while(curr){
        if(!curr->buffer)   
            fprintf(stdout, "Elemento con buffer vuoto nella tabella hash: %s\n", curr->path);
        else
            fprintf(stdout, "Elemento con buffer non vuoto nella tabella hash: %s\n", curr->path);
        curr = curr->next;
    }
}