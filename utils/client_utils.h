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


#endif