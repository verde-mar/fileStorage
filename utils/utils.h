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
 * @brief Restituisce il path di file_name
 * 
 * @param path Path assoluto
 * @param file_name Nome del file di cui restituire il path
 * @return const char* Path assoluto di file_name
 */
const char *nPath(const char* path, char *file_name);

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
int caller_write(const char *pathname, char* directory);

#endif