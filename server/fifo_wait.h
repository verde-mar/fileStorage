#ifndef FIFO_WAIT_H_
#define FIFO_WAIT_H_

/**
 * @brief Coda dei client in attesa su un nodo
 * 
 */
typedef struct clients_waiting {
    int elements;           
    struct client* head;
} clients_in_wait;

/**
 * @brief Client in attesa
 * 
 */
typedef struct client {
    int file_descriptor;
    struct client *next;
} client;

/**
 * @brief Crea la lista di attesa di un nodo
 * 
 * @param list Lista di attesa
 * @return int 0 in caso di successo, -1 altrimenti
 */
int create_list_wait(clients_in_wait **list);

/**
 * @brief Elimina la lista di attesa di un nodo
 * 
 * @param queue Coda di attesa
 * @return int 0 in caso di successo, -1 altrimenti
 */
int delete_list_wait(clients_in_wait **queue);

/**
 * @brief Aggiunge un client alla lista di attesa
 * 
 * @param file_d File descriptor del client da aggiungere nella lista di attesa
 * @param list Lista di attesa
 * @return int 0 in caso di successo, -1 altrimenti
 */
int add_list_wait(int file_d, clients_in_wait* list);

/**
 * @brief Preleva dalla lista di attesa il primo client
 * 
 * @param head_client Primo client nella lista di attesa
 * @param list Lista di attesa
 * @return int 0 in caso di successo, -1 altrimenti
 */
int del_list_wait(client **head_client, clients_in_wait* list);

#endif