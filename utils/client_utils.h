#ifndef CLIENT_UTILS_H_
#define CLIENT_UTILS_H_

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
 * @param dirname Path della directory in cui salvare i file appena letti
 * @return int 0 in caso di successo, -1 altrimenti
 */
int caller_write(const char* pathname, const char *dirname);

/**
 * @brief Legge il file da memorizzare su disco
 * 
 * @param byte_letti Byte letti fino ad ora
 * @param byte_scritti Byte scritti fino ad ora
 * @param size_path Size del path da memorizzare
 * @param path Path del file da memorizzare
 * @param old_file Buffer contenente i dati del file da memorizzare
 * @param size_old Size del file da memorizzare
 * @return int 0 in caso di successo, -1 altrimenti
 */
int freed(int *byte_letti, int *byte_scritti, size_t size_path, char** path, void** old_file, size_t *size_old);

#endif