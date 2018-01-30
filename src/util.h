#include <stdlib.h>
#include "jsmn.h"
#ifndef UTIL_H
#define UTIL_H

#define URL_SIZE 256

typedef struct {
    char* data;
    size_t size;
} result_t;

int init_curl();

int request(char (* url)[URL_SIZE], size_t urlc, result_t* res);

int parse_json(const result_t* res, jsmntok_t** tokens);
#endif
