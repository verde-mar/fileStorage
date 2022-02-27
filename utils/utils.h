#ifndef UTILS_H_
#define UTILS_H_

/**
 * @brief Verifica se il file identificato da path e' regolare
 * 
 * @param path Path assoluto del file
 * @return int 0 in caso di successo, -1 altrimenti
 */
int is_regular_file(const char *path);

/**
 * @brief 
 * 
 * @param path Path assoluto del file
 * @return int 0 in caso di successo, -1 altrimenti
 */
int is_directory(const char *path);

/**
 * @brief Salva il file path nella directory dirname
 * 
 * @param dirname Path della directory in cui salvare il file
 * @param filename Nome del file letto
 * @param buf Buffer contenente i dati del file appena letto
 * @param size Size di buf
 * @return int 0 in caso di successo, -1 altrimenti
 */
int save_on_disk(char *dirname, char *filename, char* buf, size_t size);

/**
 * @brief Chiama ricorsivamente la funziona fun sui file contenuti in pathname
 * 
 * @param fun Funzione da chiamate
 * @param pathname Path assoluto della directory
 * @return int 0 in caso di successo, -1 altrimenti
 */
int caller(int (*fun) (const char*), const char* pathname);

/**
 * @brief Legge da un file su disco
 * 
 * @param pathname Path del file
 * @param buf Buffer in cui memorizzare i dati del file
 * @param size Size di buf
 * @return int 0 in caso di successo, -1 altrimenti
 */
int read_from_file(char *pathname, char** buf, int *size);



#endif