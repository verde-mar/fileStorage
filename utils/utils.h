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
int save_on_disk(char *dirname, char *filename, void* buf, size_t size);

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
int read_from_file(char *pathname, void** buf, size_t *size);

/**
 * @brief Calcola il massimo tra due interi
 * 
 * @param a Intero di cui calcolare il massimo
 * @param b Intero di cui calcolare il massimo
 * @return int a, se e' il massimo, b altrimenti
 */
int max(int a, int b);

/**
 * @brief Restituisce il parametro bytes in Mbyte
 * 
 * @param bytes Numero byte
 * @return float bytes in Mbyte
 */
float to_Mbytes(int bytes);

/**
 * @brief Restituisce il valore del parametro in secondi
 * 
 * @param msec Millisecondi
 * @return int msec in secondi
 */
int msleep(long msec);

/**
 * @brief Tokenizza to_token e inserisce i due token rilevati in file1 e file2
 * 
 * @param to_token Stringa da tokenizzare
 * @param file1 File su cui operare
 * @param file2 File su cui operare
 * @return int 0 in caso di successo, -1 altrimenti
 */
int tokens(char *to_token, char **file1, char** file2);

#endif