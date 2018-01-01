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

#define RANK 8
#define SYMBOL 6
#define DAY_CHANGE 26
#define WEEK_CHANGE 28
#define PRICE_USD 10
#define PRICE_CON 32
#define COLUMN_SIZE 5

static int values[] = {RANK, SYMBOL, DAY_CHANGE, WEEK_CHANGE, PRICE_USD};

typedef struct {
    char* data;
    size_t size;
} result_t;

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

static int get_coins(const char* url, result_t* res) {
    CURL* curl = curl_easy_init();

    if(curl) {
        res->data = malloc(1);
        res->size = 0;

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)res);

        CURLcode err = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        if(err != CURLE_OK) {
            fprintf(stderr, "nope : %s\n", curl_easy_strerror(err));
            return -1;
        }
    } else {
        fprintf(stderr, "failed to init curl\n");
        return -2;
    }

    return 0;
}

static int print_coins(const result_t* res, const char* currency, int color_enabled) {
    static jsmn_parser parser;

    jsmn_init(&parser);
    jsmntok_t* tokens = malloc(sizeof(jsmntok_t) * TKN_SIZE);
    size_t count = 1;
    int actual;
    while ((actual = jsmn_parse(&parser, res->data, res->size, tokens, count * TKN_SIZE)) == JSMN_ERROR_NOMEM) {
        tokens = realloc(tokens, ++count * sizeof(jsmntok_t) * TKN_SIZE);
    }
    if (actual < 0) {
        free(tokens);
        fprintf(stderr, "failed to parse json with error code %d\n", actual);
        return -1;
    }

    // check if error returned
    if (strncmp(res->data + tokens[1].start, "error", 5) == 0) {
        free(tokens);
        fprintf(stderr, "%.*s\n", tokens[2].end - tokens[2].start, res->data + tokens[2].start);
        return -2;
    }

    if (color_enabled) {
        printf(COLOR_YELLOW "RANK\tSYMBOL\t24H\t7D\tPRICE (%s)\n" COLOR_RESET, currency);
    } else {
        printf("RANK\tSYMBOL\t24H\t7D\tPRICE (%s)\n", currency);
    }

    for (int i = 0; i < actual; i++) {
        if (tokens[i].type == JSMN_OBJECT) {
            for (size_t j = 0; j < COLUMN_SIZE; j++) {
                const jsmntok_t* val = tokens + i + values[j];
                char* str = res->data + val->start;
                int len = val->end - val->start;
                char* color = "";
                if (j == 2 || j == 3) {
                    if (res->data[val->start] == '-') {
                        color = COLOR_RED;
                    } else if (res->data[val->start] == 'n') {
                        str = "-";
                        len = 1;
                        color = COLOR_YELLOW;
                    } else {
                        str--;
                        len++;
                        *str = ' ';
                        color = COLOR_GREEN;
                    }
                }

                if (color_enabled) {
                    printf("%s%.*s" COLOR_RESET "\t", color, len, str);
                } else {
                    printf("%.*s\t", len, str);
                }
            }
            printf("\n");
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
    char url[URL_SIZE];
    if (args->specific) {
        snprintf(url, URL_SIZE, "https://api.coinmarketcap.com/v1/ticker/%s/?convert=%s",
                args->symbol, args->convert);
    } else {
        snprintf(url, URL_SIZE, "https://api.coinmarketcap.com/v1/ticker/?start=%lu&limit=%lu&convert=%s",
                args->start, args->limit, args->convert);
    }

    int err = get_coins(url, &res);
    if (err) {
        return err;
    }

    if (args->convert[0] == '\0') {
        values[4] = PRICE_USD;
        err = print_coins(&res, "USD", args->color_enabled);
    } else {
        values[4] = PRICE_CON;
        err = print_coins(&res, args->convert, args->color_enabled);
    }

    free(res.data);
    return err;
}

