#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "jsmn/jsmn.h"

#define TKN_SIZE 1024

#define RANK 8
#define SYMBOL 6
#define DAY_CHANGE 26
#define WEEK_CHANGE 28
#define VALUE 32
#define COLUMN_SIZE 5

static int values[COLUMN_SIZE] = {RANK, SYMBOL, DAY_CHANGE, WEEK_CHANGE, VALUE};

typedef struct {
    char* data;
    size_t size;
} result_t;

int get_coins(const char* url, result_t* res);
int print_coins(const result_t* result);

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

int main(int argc, char* argv[]) {

    result_t res;
    const char* url = "https://api.coinmarketcap.com/v1/ticker/?limit=10&convert=EUR";

    int err = get_coins(url, &res);
    if (err) {
        return err;
    }

    err = print_coins(&res);
    if (err) {
        return err;
    }

    free(res.data);
    return 0;
}

int get_coins(const char* url, result_t* res) {
    curl_global_init(CURL_GLOBAL_DEFAULT);

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

    curl_global_cleanup();

    return 0;
}

int print_coins(const result_t* res) {
    static jsmn_parser parser;

    jsmn_init(&parser);
    jsmntok_t tokens[TKN_SIZE];
    int actual = jsmn_parse(&parser, res->data, res->size, tokens, TKN_SIZE);
    if (actual < 0) {
        fprintf(stderr, "failed to parse json with error code %d\n", actual);
        return -1;
    }

    for (size_t i = 0; i < actual; i++) {
        if (tokens[i].type == JSMN_OBJECT) {
            for (size_t j = 0; j < COLUMN_SIZE; j++) {
            const jsmntok_t* val = tokens + i + values[j];
            printf("%.*s\t",
                    val->end - val->start, res->data + val->start);

            }
            printf("\n");
        }
    }

    return 0;
}

