#include "coin.h"
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include "jsmn.h"

#define COLOR_RED    "\x1b[31m"
#define COLOR_GREEN  "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET  "\x1b[0m"

#define TKN_SIZE 1024
#define URL_SIZE 256

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
    char* data;
    size_t size;
} result_t;

typedef struct {
    char* header;
    size_t offset;
    int padding;
    int use_color;
    int right_align;
} column_t;


static size_t get_callback(void* contents, size_t size, size_t nmemb, void* userdata) {
    const size_t realSize = size * nmemb;
    result_t* res = (result_t*) userdata;
    res->data = realloc(res->data, res->size + realSize + 1);
    if (!res->data) {
        fprintf(stderr, "out of memory!\n");
        return 0;
    }

    memcpy(&(res->data[res->size]), contents, realSize);
    res->size += realSize;
    res->data[res->size] = 0;

    return realSize;
}

static int get_coins(char url[MAX_SYM][URL_SIZE], size_t urlc, result_t* res) {
    CURL* curl = curl_easy_init();

    if(curl) {
        res->data = malloc(1);
        res->size = 0;

        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)res);

        for (size_t i = 0; i < urlc; i++) {
            curl_easy_setopt(curl, CURLOPT_URL, url[i]);

            CURLcode err = curl_easy_perform(curl);
            if(err != CURLE_OK) {
                fprintf(stderr, "nope : %s\n", curl_easy_strerror(err));
                return -1;
            }
        }

        curl_easy_cleanup(curl);

    } else {
        fprintf(stderr, "failed to init curl\n");
        return -2;
    }

    return 0;
}

static int parse_json(const result_t* res, jsmntok_t** tokens) {
    static jsmn_parser parser;

    jsmn_init(&parser);
    *tokens = malloc(sizeof(jsmntok_t) * TKN_SIZE);
    size_t count = 1;
    int size;
    while ((size = jsmn_parse(&parser, res->data, res->size, *tokens, count * TKN_SIZE)) == JSMN_ERROR_NOMEM) {
        *tokens = realloc(*tokens, ++count * sizeof(jsmntok_t) * TKN_SIZE);
    }
    if (size < 0) {
        free(*tokens);
        fprintf(stderr, "failed to parse json with error code %d\n", size);
        return -1;
    }

    // check if error returned
    if (strncmp(res->data + (*tokens)[1].start, "error", 5) == 0) {
        free(*tokens);
        fprintf(stderr, "%.*s\n", (*tokens)[2].end - (*tokens)[2].start, res->data + (*tokens)[2].start);
        return -2;
    }

    return size;
}

static int print_coins(const result_t* res, const column_t columns[], size_t size, int color_enabled) {
    // parse json
    jsmntok_t* tokens = NULL;
    int token_size = parse_json(res, &tokens);
    if (token_size < 0 || tokens == NULL) {
        return -1;
    }

    // print table header
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

static void cleanup() {
    curl_global_cleanup();
}

void coin_init() {
    curl_global_init(CURL_GLOBAL_DEFAULT);

    atexit(cleanup);
}

int display_result(const arguments* args) {
    result_t res;
    char urls[MAX_SYM][URL_SIZE];
    size_t urlc = 0;
    if (args->specific) {
        while (urlc < args->specific) {
            snprintf(urls[urlc], URL_SIZE, "https://api.coinmarketcap.com/v1/ticker/%s/?convert=%s",
                args->symbol[urlc], args->convert);
            ++urlc;
        }
    } else {
        snprintf(urls[0], URL_SIZE, "https://api.coinmarketcap.com/v1/ticker/?start=%lu&limit=%lu&convert=%s",
                args->start, args->limit, args->convert);
        urlc = 1;
    }

    int err = get_coins(urls, urlc, &res);
    if (err) {
        return err;
    }

    if (args->convert[0] == '\0') {
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

