#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "jsmn/jsmn.h"

#define TKN_SIZE 256
static jsmn_parser parser;

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

int main(int argc, char* argv[]) {

    curl_global_init(CURL_GLOBAL_DEFAULT);
    jsmn_init(&parser);

    CURL* curl = curl_easy_init();
    if(curl) {
        result_t res;
        res.data = malloc(1);
        res.size = 0;

        curl_easy_setopt(curl, CURLOPT_URL, "https://api.coinmarketcap.com/v1/ticker/?limit=20");
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&res);

        CURLcode err = curl_easy_perform(curl);

        if(err != CURLE_OK) {
            fprintf(stderr, "nope : %s\n", curl_easy_strerror(err));
        } else {
            printf("%s\n", res.data);
        }

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}
