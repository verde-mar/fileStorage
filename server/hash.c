#include <stdio.h>
#include <hash.h>
#include <check_errors.h>

#include <stdlib.h>
#include <string.h>

unsigned long hash_function(char *str){
    unsigned long hash = 5381;
    int c = *str++;

    while (c){
        hash += + c;
        c = *str++;
    }
    return (hash%16);
}

int create_hashtable(size_t size){
    /* Inizializza la struttura dati della tabella */
    table = (hashtable*) malloc(sizeof(hashtable));
    CHECK_OPERATION(table == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    /* Inizializza l'array delle liste di trabocco */
    table->queue = malloc(sizeof(list_t*)*16);
    CHECK_OPERATION(table->queue == NULL,
        fprintf(stderr, "Allocazione non andata a buon fine.\n");
            return -1);

    /* Inizializza le liste di trabocco*/
    for (int i = 0; i < 16; i++) {
        int err = create_list(&(table->queue[i])); 
        CHECK_OPERATION(err == -1, return -1);
    }

    /* Inizia la coda FIFO */
    int create = create_fifo(&fifo_queue);
    CHECK_OPERATION(create == -1,
        fprintf(stderr, "Errore nella creazione della coda FIFO.\n");
            return -1);

    /* Inizializza la size corrente */
    table->curr_size = 0;
    /* Inizializza la size massima della tabella hash */
    table->max_size = size;
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

    /* Elimina la coda FIFO */
    int del = delete_fifo(&fifo_queue);
    CHECK_OPERATION(del == -1,
        fprintf(stderr, "Errore nella creazione della coda FIFO.\n");
            return -1);

    /* Elimina la tabella hash */
    free(table);

    return 0;
}

int add_hashtable(char *name_file, int fd, int flags){
    CHECK_OPERATION(name_file==NULL || fd<0, 
        fprintf(stderr, "Parametro non valido.\n");
            return -1);

    /* Aggiunge l' elemento nella tabella hash */
    int success = -1;
    int hash = hash_function(name_file); 

    success = add(&(table->queue[hash]), name_file, fd, flags);   
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nell'inserimento di un elemento nella tabella hash.\n"); 
            return -1);


    return success;    
}

int del_hashtable(char *name_file, node **just_deleted, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);

    int hash = hash_function(name_file);
   
    /* Elimina un nodo */
    int success = deletes(&(table->queue[hash]), name_file, just_deleted, fd);
    CHECK_OPERATION(success == -1, 
        fprintf(stderr, "Errore nell'eliminazione di un elemento nella tabella hash.\n"); 
            return -1);
    
    return success;
}

int close_hashtable(char *name_file, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);
    int success = -1;
    int hash = hash_function(name_file); 
   
    /* Chiude un nodo */
    success = closes(&(table->queue[hash]), name_file, fd);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella chiusura di un elemento nella tabella hash.\n"); 
            return -1);
    
    return success;
}

int unlock_hashtable(char *name_file, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);
    int success = -1;
    int hash = hash_function(name_file); 

    /* Rilascia la lock di un nodo */
    success = unlock(&(table->queue[hash]), name_file, fd);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nel reset della lock di un elemento nella tabella hash.\n"); 
            return -1);

    return success;
}

int lock_hashtable(char *name_file, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);
    int success = -1;
    int hash = hash_function(name_file); 

    /* Acquisisce la lock di un nodo */
    success = lock(&(table->queue[hash]), name_file, fd);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nel set della lock di un elemento nella tabella hash.\n"); 
            return -1);

    return success;
}

int read_hashtable(char *name_file, char** buf, int fd){
    CHECK_OPERATION(name_file==NULL || fd<0,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);
    int success = -1;
    int hash = hash_function(name_file); 

    /* Legge i dati di un nodo */
    success = reads(&(table->queue[hash]), name_file, buf, fd);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella lettura di un elemento nella tabella hash.\n"); 
            return -1);

    return success;
}

int append_hashtable(char* name_file, char* buf, node** deleted, int fd){
    CHECK_OPERATION((name_file==NULL || fd<0) || buf==NULL ,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);
    int success = -1;
    int hash = hash_function(name_file);
     
    /* Se prima non e' stata fatta la writeFile, viene restituito un errore */
    CHECK_OPERATION(buf == NULL, return 707);

    /* Effettua la append su un nodo */
    success = append_buffer(&(table->queue[hash]), name_file, buf, &(table->max_size), &(table->curr_size), deleted, fd);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella append su un elemento nella tabella hash.\n"); 
            return -1);

    return success;
}

int write_hashtable(char* name_file, char* buf, node** deleted, int fd){
    CHECK_OPERATION((name_file==NULL || fd<0) || buf==NULL ,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);
    int success = -1;
    int hash = hash_function(name_file);
    /* Effettua la write su un nodo */
    success = writes(&(table->queue[hash]), name_file, buf, &(table->max_size), &(table->curr_size), deleted, fd);
    CHECK_OPERATION(success==-1, 
        fprintf(stderr, "Errore nella scrittura di un elemento nella tabella hash.\n"); 
            return -1);

    return success;
}

int readN_hashtable(int N, char** buf, int fd){
    CHECK_OPERATION(fd<0,
        fprintf(stderr, "Parametri non validi.\n");
            return -1;);
    int success = -1;

    node_c *curr = fifo_queue->head;
    while(curr && N>0){
        int hash = hash_function((char*)curr->path); 
    
        /* Legge i dati di un nodo */
        success = reads(&(table->queue[hash]), (char*)curr->path, buf, fd);
        CHECK_OPERATION(success==-1, 
            fprintf(stderr, "Errore nella lettura di un elemento nella tabella hash.\n"); 
                return -1);
        curr = curr->next;
        N--;
    }
    fprintf(stdout, "Non ci sono elementi da leggere.\n");
    return success;
}