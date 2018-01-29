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
    int color_enabled;
} arguments;

int coin_init();

int display_result(const arguments* args);

#endif

