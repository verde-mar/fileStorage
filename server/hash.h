#ifndef HASH_H_
#define HASH_H_
#include <queue.h>

/**
 * @brief Tabella hash
 * 
 */
typedef struct hashtable {
    struct list **files;
    struct list **queue;
    int size;
} hashtable;

hashtable *table;

/**
 * @brief Crea la tabella hash
 * 
 * @return hashtable* La tabella hash
 */
hashtable* create_hashtable (int num_file);

#endif