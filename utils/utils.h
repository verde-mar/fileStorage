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
 * @brief Chiama la openFile ricorsivamente
 * 
 * @param path Path assoluto del file
 * @return int 0 in caso di successo, -1 altrimenti
 */
int caller_open(const char *pathname);

/**
 * @brief Chiama la writeFile ricorsivamente
 * 
 * @param path Path assoluto del file
 * @param directory Path della directory in cui salvare i file appena letti
 * @return int 0 in caso di successo, -1 altrimenti
 */
//int caller_write(const char *pathname, char* directory);

/**
 * @brief Salva il file path nella directory dirname
 * 
 * @param dirname Path della directory in cui salvare il file
 * @param path Path assoluto del file letto
 * @param buf Buffer contenente i dati del file appena letto
 * @param size Size di buf
 * @return int 0 in caso di successo, -1 altrimenti
 */
int save_on_disk(char *dirname, const char* path, char* buf, size_t size);

#endif