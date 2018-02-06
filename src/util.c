#include "util.h"
#include <curl/curl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define TKN_SIZE 1024
#define CURR_SIZE 31

static CURL* curl;
static int initialized = 0;

static const char* currencies[] = {
    "AUD", "BRL", "CAD", "CHF", "CLP", "CNY", "CZK", "DKK",
    "EUR", "GBP", "HKD", "HUF", "IDR", "ILS", "INR", "JPY",
    "KRW", "MXN", "MYR", "NOK", "NZD", "PHP", "PKR", "PLN",
    "RUB", "SEK", "SGD", "THB", "TRY", "TWD", "ZAR" };

static void cleanup() {
    curl_global_cleanup();
    curl_easy_cleanup(curl);
    initialized = 0;
}

static size_t get_callback(void* contents, size_t size, size_t nmemb, void* userdata) {
    const size_t real_size = size * nmemb;
    result_t* res = (result_t*) userdata;
    res->data = realloc(res->data, res->size + real_size + 1);
    if (!res->data) {
        fprintf(stderr, "out of memory!\n");
        return 0;
    }

    memcpy(&(res->data[res->size]), contents, real_size);
    res->size += real_size;
    res->data[res->size] = 0;

    return real_size;
}

int init_curl() {
    if (initialized) {
        return 0;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "failed to init libcurl\n");
        return -1;
    }
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_callback);

    atexit(cleanup);
    initialized = 1;
    return 0;
}

int is_available(char* curr) {
    if (strlen(curr) == 3) {
        curr[0] = toupper(curr[0]);
        curr[1] = toupper(curr[1]);
        curr[2] = toupper(curr[2]);
        for (size_t i = 0; i < CURR_SIZE; i++) {
            if (curr[0] == currencies[i][0]
                    && curr[1] == currencies[i][1]
                    && curr[2] == currencies[i][2]) {
                return 1;
            }
        }
    }
    return 0;
}

int request(char (* url)[URL_SIZE], size_t urlc, result_t* res) {
    if (!initialized && init_curl()) {
        return -1;
    }

    res->data = NULL;
    res->size = 0;

    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)res);

    for (size_t i = 0; i < urlc; i++) {
        curl_easy_setopt(curl, CURLOPT_URL, url[i]);

        CURLcode err = curl_easy_perform(curl);

        if(err == CURLE_OK) {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (response_code == 404) {
                free(res->data);
                res->size = 0;
                fprintf(stderr, "id not found\n");
                return -1;
            }
        } else {
            fprintf(stderr, "nope : %s\n%s", curl_easy_strerror(err), url[i]);
            return -1;
        }
    }

    return 0;
}

int parse_json(const result_t* res, jsmntok_t** tokens) {
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

    return size;
}

