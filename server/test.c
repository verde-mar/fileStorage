#include <queue.h>
#include <fifo.h>
#include <stdio.h>

#include <stdlib.h>

void *myfun(void *arg){
    
    return NULL;
}

void *myfundelete(void *arg){
    char *name = remove_fifo();
    printf("nome della testa: %s\n", name);

    return NULL;
}

int main(int argc, char const *argv[])
{
    pthread_t tid;
    int err, status;
    create_fifo();

    if((err=pthread_create(&tid, NULL, &myfun, "micio"))!=0){
        printf("errore\n");
    } else {
        pthread_join(tid, (void*)&status);
    }

    pthread_t tid1;
    int err1;
    if((err1=pthread_create(&tid1, NULL, &myfun, "cane"))!=0){
        printf("errore\n");
    } else {
        pthread_join(tid1, (void*)&status);
    }

    pthread_t tid2;
    int err2;
    if((err2=pthread_create(&tid2, NULL, &myfundelete, "cane"))!=0){
        printf("errore\n");
    } else {
        pthread_join(tid2, (void*)&status);
    }

    node_c* current = fifo_queue->head;
    while(current!=NULL){
        printf("current->path: %s\n", current->path);
        current = current->next;
    }
    delete_fifo();

    return 0;
}
