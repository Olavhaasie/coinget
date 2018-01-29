#include <stdlib.h>
#ifndef COIN_H
#define COIN_H

#define MAX_SYM 10

typedef struct {
    size_t start;
    size_t limit;
    char* convert;
    char* symbol[MAX_SYM];
    int specific;
    int color_enabled;
} arguments;

void coin_init();

int display_result(const arguments* args);

#endif

