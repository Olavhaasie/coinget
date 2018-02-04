#include "coin.h"
#include <stdio.h>
#include "util.h"

#define COLOR_RED    "\x1b[31m"
#define COLOR_GREEN  "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET  "\x1b[0m"

#define RANK        { "RANK", 8, 4, 0, 1 }
#define SYMBOL      { "SYMBOL", 6, 8, 0, 0 }
#define DAY_CHANGE  { "24H (%)", 26, 7, 1, 0 }
#define WEEK_CHANGE { "7D (%)", 28, 7, 1, 0 }
#define PRICE_USD   { "PRICE (USD)", 10, 12, 0, 0 }
#define PRICE_CON   { "PRICE", 32, 12, 0, 0 }

#define HOR_SEP "|"
#define VER_SEP "-"
#define CROSS_SEP "+"


typedef struct {
    char* header;
    size_t offset;
    int padding;
    int use_color;
    int right_align;
} column_t;


static void print_header(const column_t columns[], size_t size, int color_enabled) {
    if (color_enabled) printf(COLOR_YELLOW);
    for (size_t i = 0; i < size; i++) {
        const char* format = columns[i].right_align ? "%*s" : "%-*s";
        printf(format, columns[i].padding, columns[i].header);
        if (i < size - 1U) {
            printf(" " HOR_SEP " ");
        }
    }
    printf("\n");
    for (size_t i = 0; i < size; i++) {
        for (int j = 0; j < columns[i].padding; j++) {
            printf(VER_SEP);
        }
        if (i < size - 1U) {
            printf(VER_SEP CROSS_SEP VER_SEP);
        }
    }
    if (color_enabled) printf(COLOR_RESET);
    printf("\n");
}

static int print_coins(const result_t* res, const column_t columns[], size_t size, int color_enabled) {
    // parse json
    jsmntok_t* tokens = NULL;
    int token_size = parse_json(res, &tokens);
    if (token_size < 0 || tokens == NULL) {
        return -1;
    }

    print_header(columns, size, color_enabled);

    // print table rows
    for (int i = 0; i < token_size; i++) {
        if (tokens[i].type == JSMN_OBJECT) {
            for (size_t j = 0; j < size; j++) {
                const jsmntok_t* val = tokens + i + columns[j].offset;
                char* str = res->data + val->start;
                int len = val->end - val->start;
                char* color = "";
                const char* format = columns[j].right_align ? "%*.*s" : "%-*.*s";

                if (columns[j].use_color) {
                    color = str[0] == '-' ? COLOR_RED : COLOR_GREEN;
                }
                if (val->type == JSMN_PRIMITIVE && str[0] == 'n') {
                    str[0] = 'x';
                    len = 1;
                    color = COLOR_YELLOW;
                }

                if (color_enabled) printf("%s", color);
                printf(format, columns[j].padding, len, str);
                if (color_enabled) printf(COLOR_RESET);
                if (j < size - 1U) {
                    printf(" " HOR_SEP " ");
                }
            }
            printf("\n");
            i += 2 * tokens[i].size;
        }
    }

    free(tokens);
    return 0;
}

int display_result(const arguments* args) {
    result_t res;
    char (* urls)[URL_SIZE];
    size_t urlc = 0;
    if (args->specific) {
        urls = malloc(args->specific * URL_SIZE * sizeof(char*));
        while (urlc < args->specific) {
            snprintf(urls[urlc], URL_SIZE, "https://api.coinmarketcap.com/v1/ticker/%s/?convert=%s",
                args->symbols[urlc], args->convert);
            ++urlc;
        }
    } else {
        urls = malloc(URL_SIZE * sizeof(char*));
        snprintf(urls[0], URL_SIZE, "https://api.coinmarketcap.com/v1/ticker/?start=%lu&limit=%lu&convert=%s",
                args->start, args->limit, args->convert);
        urlc = 1;
    }

    int err = request(urls, urlc, &res);
    free(urls);
    if (err) {
        return err;
    }

    if (!args->convert) {
        column_t columns[] = { RANK, SYMBOL, DAY_CHANGE, WEEK_CHANGE, PRICE_USD };
        err = print_coins(&res, columns, 5, args->color_enabled);
    } else {
        column_t columns[] = { RANK, SYMBOL, DAY_CHANGE, WEEK_CHANGE, PRICE_CON };
        char header[16];
        snprintf(header, 16, "PRICE (%s)", args->convert);
        columns[4].header = header;

        err = print_coins(&res, columns, 5, args->color_enabled);
    }

    free(res.data);
    return err;
}

