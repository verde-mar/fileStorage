#include <queue.h>
#include <stdlib.h>
#include <stdio.h>

#include <check_errors.h>
#include <string.h>
#include <errno.h>

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
        PTHREAD_DESTROY_LOCK(tmp->mutex);
        free(tmp->mutex);
        if(tmp->buffer)
            free(tmp->buffer);
        free((char*)tmp->path);

        client *in_wait = NULL;
        while((tmp->waiting_list)->head){
            in_wait = tmp->waiting_list->head;
            (tmp->waiting_list)->head = ((tmp->waiting_list)->head)->next;
            free(in_wait);
        }
        free(tmp->waiting_list);

        free(tmp);
    }
    /* Distrugge la lock della lista di trabocco */
    PTHREAD_DESTROY_LOCK((*lista_trabocco)->mutex);
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
 * @return node* Nodo creato o NULL
 */
static node* node_create(list_t **queue, char* file_path, int fd){
    CHECK_OPERATION(!(*queue),
        fprintf(stderr, "Parametri non validi.\n");
        return NULL);

    /* Crea il nodo */
    node *curr = (node*)malloc(sizeof(node));
    CHECK_OPERATION(curr == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);

    /* Inizializza la maschera */
    FD_ZERO(&(curr->open));
    /* Apre il file */
    FD_SET(fd, &(curr->open));

    /* Inizializza il path del file */
    curr->path = malloc(sizeof(char)*(strlen(file_path)+1));
    CHECK_OPERATION(curr->path == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    strcpy((char*)curr->path, file_path);

    /* Inizializza la size del buffer e il buffer */
    curr->size_buffer = 0;
    curr->buffer = NULL;
    /* Inizializza la lock logica */
    curr->fd_c = -1;
    /* Inizializza il flag che indica se l'ultima operazione e' stata la create_lock */
    curr->fd_create_open = -1;
    /* Crea la lista di attesa */
    int err_wait_list = create_list_wait(&(curr->waiting_list));
    CHECK_OPERATION(err_wait_list == -1, fprintf(stderr, "Errore nella creazione della lista di attesa del nodo.\n"); return NULL);

    /* Inizializza la mutex */
    curr->mutex = malloc(sizeof(pthread_mutex_t));
    CHECK_OPERATION(curr->mutex == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); return NULL);
    PTHREAD_INIT_LOCK(curr->mutex);

    return curr;
}

/**
 * @brief Acquisisce la lock sul nodo, altrimenti si mette in attesa
 * 
 * @param nodo Nodo su cui acquisire la lock
 * @param fd File descriptor del client che vuole acquisire la lock
 * @return int 0 in caso di successo, -1 altrimenti
 */
static int lock_acquire(node *nodo, int fd, FILE *file_log){
    CHECK_OPERATION(nodo == NULL || fd < 0, fprintf(stderr, "Parametri non validi.\n"); return -1);

    /* Se possiede gia' la lock */
    if(nodo->fd_c == fd) {
        return 0;
    }

    /* Se la lock e' libera */
    if(nodo->fd_c == -1){
        nodo->fd_c = fd;
        return 0;
    }
    /* Altrimenti, il client viene messo in lista d'attesa */
    int add_cl = add_list_wait(fd, nodo->waiting_list);
    CHECK_OPERATION(add_cl == -1, fprintf(stderr, "Errore nella aggiunta di un file nella lista di attesa.\n"); return -1);


    return 1;
}

int creates(list_t **lista_trabocco, char* file_path, int fd, atomic_int *max_file_reached, FILE* file_log){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Cerca per verificare se il file esiste gia' nella tabella */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo != NULL, return 101);
    CHECK_OPERATION(nodo == NULL, 
        nodo = node_create(lista_trabocco, file_path, fd);
        if(nodo == NULL) return -1);

    PTHREAD_LOCK(fifo_queue->mutex);
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    
    /* Inserisci il file nella coda cache */
    int adder = add_fifo((char*)nodo->path);
    CHECK_OPERATION(adder == -1, 
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);
        return -1);
    
    /* Aggiorna il numero di file presenti nella tabella hash */
    *max_file_reached = max(*max_file_reached, fifo_queue->elements);

    /* Aggiunge effettivamente il nodo alla tabella hash */
    nodo->next = (*lista_trabocco)->head; 
    (*lista_trabocco)->head = nodo;

    /* Aggiorna il file di log */
    fprintf(file_log, "Create\n");

    PTHREAD_UNLOCK((*lista_trabocco)->mutex);
    PTHREAD_UNLOCK(fifo_queue->mutex);

    return 0;
}

int creates_locks(list_t **lista_trabocco, char* file_path, int fd, atomic_int *max_file_reached, FILE* file_log){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Cerca per verificare se il file esiste gia' nella tabella */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo != NULL, return 101);
    CHECK_OPERATION(nodo == NULL, 
        nodo = node_create(lista_trabocco, file_path, fd);
        if(nodo == NULL) return -1);
    /* Setta la lock */
    nodo->fd_c = fd;
    /* Setta il flag che indica se l'operazione immediatamente precedente e' stata la create_lock */
    nodo->fd_create_open = fd;
    
    PTHREAD_LOCK(fifo_queue->mutex);
    PTHREAD_LOCK((*lista_trabocco)->mutex);
    
    /* Inserisci il file nella coda cache */
    int adder = add_fifo((char*)nodo->path);
    CHECK_OPERATION(adder == -1, 
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);
        return -1);
    
    /* Aggiorna il numero di file presenti nella tabella hash */
    *max_file_reached = max(*max_file_reached, fifo_queue->elements);

    /* Aggiunge effettivamente il nodo alla tabella hash */
    nodo->next = (*lista_trabocco)->head; 
    (*lista_trabocco)->head = nodo;

    /* Aggiorna il file di log */
    fprintf(file_log, "Create_Lock\n");

    PTHREAD_UNLOCK((*lista_trabocco)->mutex);
    PTHREAD_UNLOCK(fifo_queue->mutex);

    return 0;
}

int opens_locks(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Cerca per verificare se il file esiste nella tabella */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL, return 505);
    
    PTHREAD_LOCK(nodo->mutex);

    int err_lock = lock_acquire(nodo, fd, file_log);
    CHECK_OPERATION(err_lock == -1, fprintf(stderr, "Errore nella acquisizione della lock nel nodo %s\n", nodo->path); PTHREAD_UNLOCK(nodo->mutex); return -1);
    /* Apre il file */
    FD_SET(fd, &(nodo->open));

    /* Aggiorna il file di log */
    fprintf(file_log, "Open_Lock\n");

    PTHREAD_UNLOCK(nodo->mutex);

    return err_lock;
}

int opens(list_t **lista_trabocco, char* file_path, int fd, FILE* file_log){
    CHECK_OPERATION(!(*lista_trabocco),
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Cerca per verificare se il file esiste gia' nella tabella */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL, return 505);
    
    PTHREAD_LOCK(nodo->mutex);

    /* Apre il file */
    FD_SET(fd, &(nodo->open));

    /* Aggiorna il file di log */
    fprintf(file_log, "Open\n");

    PTHREAD_UNLOCK(nodo->mutex);

    return 0;
}

int deletes(list_t **lista_trabocco, char* file_path, node** just_deleted, int fd, atomic_int *curr_size, FILE* file_log){
    CHECK_OPERATION(*lista_trabocco==NULL,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    PTHREAD_LOCK(fifo_queue->mutex);
    PTHREAD_LOCK((*lista_trabocco)->mutex);

    /* Rimuove l'elemento dalla coda fifo */
    int remover = del(file_path);

    /* Se non c'erano elementi da eliminare */
    CHECK_OPERATION(remover == -1, 
        PTHREAD_UNLOCK(fifo_queue->mutex);
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        return 333);
    
    node* curr; /* Puntatore al nodo corrente */
    if ((*lista_trabocco)->head == NULL){ /* Lista vuota */
        fprintf(file_log, "Delete\n"); 
        PTHREAD_UNLOCK((*lista_trabocco)->mutex);
        PTHREAD_UNLOCK(fifo_queue->mutex);

        return 333;
    }

    curr = (*lista_trabocco)->head;
    if(curr){
        if (strcmp(curr->path, file_path) == 0) { /* Cancellazione del primo nodo */
            PTHREAD_LOCK(curr->mutex);
            *just_deleted = curr;
            (*lista_trabocco)->head = curr->next; /* Aggiorna il puntatore alla testa */
            *curr_size = *curr_size - curr->size_buffer;

            FD_CLR(fd, &(curr->open));
            fprintf(file_log, "Delete\n");

            PTHREAD_UNLOCK(curr->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);
            PTHREAD_UNLOCK(fifo_queue->mutex);

            return 0;
        } 
    }
    
    /* Scansione della lista a partire dal secondo nodo */
    node* prev = curr;
    curr = curr->next;
    while (curr != NULL){
        if (strcmp(curr->path, file_path) == 0) { /* Cancellazione del primo nodo */

            PTHREAD_LOCK(curr->mutex);
            *just_deleted = curr;/* Aggiorna il puntatore alla testa */
            *curr_size = *curr_size - curr->size_buffer;
            prev->next = curr->next;

            FD_CLR(fd, &(curr->open));
            fprintf(file_log, "Delete\n");

            PTHREAD_UNLOCK(curr->mutex);
            PTHREAD_UNLOCK((*lista_trabocco)->mutex);
            PTHREAD_UNLOCK(fifo_queue->mutex);

            return 0;
        }
        prev = curr;
        curr = curr->next;
    }
    PTHREAD_UNLOCK((*lista_trabocco)->mutex);
    PTHREAD_UNLOCK(fifo_queue->mutex);
    
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
    CHECK_OPERATION(nodo == NULL, return 505);

    PTHREAD_LOCK(nodo->mutex);
    
    /* Se il nodo e' aperto */
    if(FD_ISSET(fd, &(nodo->open))){
        FD_CLR(fd, &(nodo->open));
        
        fprintf(file_log, "Close\n");

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

int unlock(list_t **lista_trabocco, char* file_path, int fd, int *fd_next, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se ha acquisito la lock */
    if(nodo->fd_c == fd){
        /* Risveglia il primo client in attesa */
        if(nodo->waiting_list->head){
            client *in_wait;
            int err_del = del_list_wait(&in_wait, nodo->waiting_list);
            CHECK_OPERATION(err_del == -1,
                fprintf(stderr, "Errore nella prelevazione della testa della coda in attesa del nodo %s\n", nodo->path);
                PTHREAD_UNLOCK(nodo->mutex);
                return -1
            );
            /* Viene data la lock al thread appena risvegliato */
            nodo->fd_c = in_wait->file_descriptor;

            *fd_next = in_wait->file_descriptor; //TODO: devi dire come funziona il risveglio
            free(in_wait);
        } 
        /* Se non c'erano thread da risvegliare */
        else {
            nodo->fd_c = -1;
            *fd_next = -1;
        }
        fprintf(file_log, "Unlock\n");
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
    CHECK_OPERATION(nodo == NULL, return 505);
                    
    PTHREAD_LOCK(nodo->mutex);

    int err_lock = lock_acquire(nodo, fd, file_log);
    CHECK_OPERATION(err_lock==-1, fprintf(stderr, "Errore nella acquisizione della lock.\n"); PTHREAD_UNLOCK(nodo->mutex); return -1);
    fprintf(file_log, "Lock\n");

    PTHREAD_UNLOCK(nodo->mutex);

    return err_lock;
}

int reads(list_t **lista_trabocco, char* file_path, void** buf, size_t *size_buf, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        *buf = NULL;
        *size_buf = 0;
        return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        *buf = NULL;
        *size_buf = 0;
        return 505);

    PTHREAD_LOCK(nodo->mutex);
    /* Se il nodo e' aperto ed e' stata acquisita la lock */
    if(FD_ISSET(fd, &(nodo->open)) && nodo->fd_c == fd){
        *size_buf = nodo->size_buffer;
        *buf = malloc(*size_buf);
        CHECK_OPERATION(*buf == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); PTHREAD_UNLOCK(nodo->mutex); return -1);
        void* check = memcpy(*buf, nodo->buffer, *size_buf);  
        CHECK_OPERATION(check == NULL, fprintf(stderr, "La memcpy della read e' fallita.\n"); PTHREAD_UNLOCK(nodo->mutex); return -1);
        
        fprintf(file_log, "Read %ld\n", nodo->size_buffer);
    } 
    /* Se la lock non e' stata acquisita da nessuno */
    else if(nodo->fd_c==-1){
        *buf = NULL;
        *size_buf = 0;
        PTHREAD_UNLOCK(nodo->mutex);

        return 555;
    }
    /* Se la lock e' stata acquisita da un altro thread */
    else if(nodo->fd_c!=fd){
        *buf = NULL;
        *size_buf = 0;
        PTHREAD_UNLOCK(nodo->mutex);

        return 202; 
    }
    /* Se il nodo e' chiuso */
    else {
        *buf = NULL;
        *size_buf = 0;
        PTHREAD_UNLOCK(nodo->mutex);

        return 303;
    }
    PTHREAD_UNLOCK(nodo->mutex);
    
    return 0;
}

int append_buffer(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, atomic_int* curr_size, atomic_int* max_size_reached, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        if(buf) free(buf);
        return -1);

    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        if(buf) free(buf);
        return 505);

    PTHREAD_LOCK(nodo->mutex);

    /* Se ha acquisito la lock e il nodo e' aperto */
    if(nodo->fd_c == fd && FD_ISSET(fd, &(nodo->open))){
        /* Effettua la append del buffer passato come parametro e aggiorna la size del nodo */
        nodo->buffer = realloc(nodo->buffer, nodo->size_buffer + size_buf); 
        CHECK_OPERATION(nodo->buffer == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); 
            PTHREAD_UNLOCK(nodo->mutex);
            free(buf);
            return -1);

        void *check = memcpy(nodo->buffer + nodo->size_buffer, buf, size_buf);
        CHECK_OPERATION(check == NULL, fprintf(stderr, "La memcpy della append e' fallita.\n"); free(buf); PTHREAD_UNLOCK(nodo->mutex); return -1);
        
        nodo->size_buffer += (size_buf); 
        /* Aggiorna la size corrente della tabella hash */
        *curr_size = *curr_size + size_buf;

        /* Aggiorna la massima size raggiunta */
        *max_size_reached = max(*curr_size, *max_size_reached);

        fprintf(file_log, "Write %ld\n", size_buf);

        if(buf) free(buf);
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
    
    PTHREAD_UNLOCK(nodo->mutex);
        
    return 0;
}

int writes(list_t **lista_trabocco, char* file_path, void* buf, size_t size_buf, int *max_size, atomic_int* curr_size, atomic_int* max_size_reached, node** deleted, int fd, FILE* file_log){
    CHECK_OPERATION(!*lista_trabocco,
        fprintf(stderr, "Parametri non validi.\n");
        if(buf) free(buf);
        return -1);
            
    /* Ricerca il nodo */
    node* nodo = look_for_node(lista_trabocco, file_path);
    CHECK_OPERATION(nodo == NULL,
        size_buf = 0;
        if(buf)free(buf);
        return 505);
    
    PTHREAD_LOCK(nodo->mutex);
    if(nodo->fd_create_open == fd){ //TODO: specificare nella relazione che solo chi crea puo' fare la write
        if(buf){
            /* Alloca la dimensione per il buffer da scrivere e lo scrive*/
            nodo->buffer = malloc(size_buf);
            CHECK_OPERATION(nodo->buffer == NULL, fprintf(stderr, "Allocazione non andata a buon fine.\n"); free(buf); PTHREAD_UNLOCK(nodo->mutex); return -1);
            void* check = memcpy(nodo->buffer, buf, size_buf);  
            CHECK_OPERATION(check == NULL, fprintf(stderr, "La memcpy della append e' fallita.\n"); free(buf); PTHREAD_UNLOCK(nodo->mutex); return -1);

            /* Aggiorna la size del nodo, quella locale e la massima raggiunta */
            nodo->size_buffer = size_buf;
            *curr_size = *curr_size + size_buf;

            *max_size_reached = max(*curr_size, *max_size_reached);

            /* Aggiorna il file di log */
            fprintf(file_log, "Write %ld\n", size_buf);
            
            nodo->fd_create_open = -1;
            free(buf);
            PTHREAD_UNLOCK(nodo->mutex);
        
            return 0;
        } 
        
    } 
    /* Se non e' stata fatta la openFile(O_CREATE | O_LOCK) */
    else {
        if(buf) free(buf);
        PTHREAD_UNLOCK(nodo->mutex);

        return 606;
    }

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