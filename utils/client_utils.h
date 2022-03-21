#ifndef CLIENT_UTILS_H_
#define CLIENT_UTILS_H_

int open_write_append(const char* rest, const char* dirnameD);

int caller_one(int (*fun) (const char*), const char* pathname);

int caller_two(int (*fun) (const char*, const char*), const char* pathname, const char* dirnameD);

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
int receiver(int *byte_letti, int *byte_scritti, size_t size_path, char** path, void** old_file, size_t *size_old);

#endif