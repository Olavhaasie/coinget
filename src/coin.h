#include <stdlib.h>
#include <string.h>
#ifndef COIN_H
#define COIN_H

typedef struct {
    size_t start;
    size_t limit;
    char* convert;
    char** symbols;
    int specific;
    int global;
    int color_enabled;
} arguments;

int display_result(const arguments* args);

int display_global(const arguments* args);

#endif

