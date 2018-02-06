#include "coin.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "util.h"

#define COLOR_RED     "\x1b[31m"
#define COLOR_GREEN   "\x1b[32m"
#define COLOR_YELLOW  "\x1b[33m"
#define COLOR_RESET   "\x1b[0m"
#define COLOR_BYELLOW "\x1b[33;1m"

#define RANK        { "RANK", 8, 4, 0, 1, 0 }
#define SYMBOL      { "SYMBOL", 6, 8, 0, 0, 0 }
#define DAY_CHANGE  { "24H (%)", 26, 7, 1, 0, 1 }
#define WEEK_CHANGE { "7D (%)", 28, 7, 1, 0, 1 }
#define PRICE_USD   { "PRICE (USD)", 10, 12, 0, 0, 1 }
#define PRICE_CON   { "PRICE", 32, 12, 0, 0, 1 }

#define HOR_SEP "|"
#define VER_SEP "-"
#define CROSS_SEP "+"

static int color_enabled = 1;

typedef struct {
    char* header;
    size_t offset;
    int padding;
    int use_color;
    int right_align;
    int is_number;
} column_t;

static void cprintf(const char* color, const char* format, ...) {
    if (color_enabled) {
        printf("%s", color);
    }
    va_list vl;
    va_start(vl, format);
    vprintf(format, vl);
    va_end(vl);
    if (color_enabled) {
        printf(COLOR_RESET);
    }
}

static void print_header(const column_t columns[], size_t size) {
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

static int print_coins(const result_t* res, const column_t columns[], size_t size) {
    // parse json
    jsmntok_t* tokens = NULL;
    int token_size = parse_json(res, &tokens);
    if (token_size < 0 || tokens == NULL) {
        return -1;
    }

    print_header(columns, size);

    // print table rows
    for (int i = 0; i < token_size; i++) {
        if (tokens[i].type == JSMN_OBJECT) {
            for (size_t j = 0; j < size; j++) {
                const jsmntok_t* val = tokens + i + columns[j].offset;
                char* str = res->data + val->start;
                int len = val->end - val->start;
                char* color = "";
                const char* format = columns[j].right_align ? "%*.*s" : "%-*.*s";
                const char* nformat = columns[j].right_align ? "%*.2f" : "%-*.2f";

                if (columns[j].use_color) {
                    color = str[0] == '-' ? COLOR_RED : COLOR_GREEN;
                }
                if (val->type == JSMN_PRIMITIVE && str[0] == 'n') {
                    str[0] = 'x';
                    len = 1;
                    color = COLOR_YELLOW;
                }

                double number = columns[j].is_number ? atof(str) : 0.0;

                if (number) {
                    cprintf(color, nformat, columns[j].padding, number);
                } else {
                    cprintf(color, format, columns[j].padding, len, str);
                }
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
    color_enabled = args->color_enabled;
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
        err = print_coins(&res, columns, 5);
    } else {
        column_t columns[] = { RANK, SYMBOL, DAY_CHANGE, WEEK_CHANGE, PRICE_CON };
        char header[16];
        snprintf(header, 16, "PRICE (%s)", args->convert);
        columns[4].header = header;

        err = print_coins(&res, columns, 5);
    }

    free(res.data);
    return err;
}

int display_global(const arguments* args) {
    color_enabled = args->color_enabled;
    char url[URL_SIZE];
    if (args->convert) {
        snprintf(url, URL_SIZE, "https://api.coinmarketcap.com/v1/global?convert=%s",
                args->convert);
    } else {
        snprintf(url, URL_SIZE, "https://api.coinmarketcap.com/v1/global");
    }

    result_t res;
    if (request(&url, 1, &res)) {
        return -1;
    }

    jsmntok_t* tokens;
    parse_json(&res, &tokens);

    size_t cur_offset = args->convert ? 16 : 2;

    cprintf(COLOR_BYELLOW, "total marketcap (%3s) | ", args->convert ? args->convert : "USD");

    printf("%.*s (%.*s%% BTC)\n",
            tokens[cur_offset].end - tokens[cur_offset].start, res.data + tokens[cur_offset].start,
            tokens[6].end - tokens[6].start, res.data + tokens[6].start);

    cprintf(COLOR_BYELLOW, "active currencies     | ");

    printf("%.*s\n", tokens[8].end - tokens[8].start, res.data + tokens[8].start);

    cprintf(COLOR_BYELLOW, "active markets        | ");

    printf("%.*s\n", tokens[12].end - tokens[12].start, res.data + tokens[12].start);

    cprintf(COLOR_BYELLOW, "timestamp             | ");

    time_t time = (time_t) atol(res.data + tokens[14].start);
    struct tm* tm = localtime(&time);
    char date[20];
    strftime(date, sizeof(date), "%b %d %Y %H:%M", tm);
    printf("%s\n", date);

    free(tokens);
    free(res.data);
    return 0;
}

