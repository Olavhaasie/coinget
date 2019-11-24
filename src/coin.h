#ifndef COIN_H
#define COIN_H
#include <stdlib.h>
#include <string.h>

#include "util.h"

typedef struct {
    size_t start;
    size_t limit;
    char* convert;
    char** symbols;
    size_t specific;
    int global;
    char* portfolio;
    int color_enabled;
} arguments;

int display_result(const arguments* args);

int display_global(const arguments* args);

int display_portfolio(const arguments* args);

#endif
