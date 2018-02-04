#include <stdlib.h>
#include "jsmn.h"
#ifndef UTIL_H
#define UTIL_H

#define URL_SIZE 256 // max url character size

typedef struct {
    char* data;
    size_t size;
} result_t;

/**
 * init the curl library. Will be automatically called by request if unitialized.
 *
 * @return 0 when successful, non-zero otherwise
 */
int init_curl();

/**
 * Make multiple http GET requests and store them.
 * Function will allocate memory for res if result is 0.
 *
 * @param url  pointer to array of strings
 * @param urlc number of given urls
 * @param res  pointer to unallocated result struct.
 * @return 0 when successful, non-zero otherwise
 */
int request(char (* url)[URL_SIZE], size_t urlc, result_t* res);

/**
 * Parse the json result from a result struct into tokens.
 * Data will be allocated if return is greater than 0.
 *
 * @param res    pointer to result struct with data
 * @param tokens pointer to pointer of uninitialized tokens
 * @return the amount of tokens parsed, less than zero when failed.
 */
int parse_json(const result_t* res, jsmntok_t** tokens);

#endif

