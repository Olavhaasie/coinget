#include <stdio.h>
#include <curl/curl.h>
#include "jsmn/jsmn.h"

#define TKN_SIZE 256
static jsmn_parser parser;


static size_t get_callback(void* content, size_t size, size_t nmemb, void* userdata) {
    const size_t realSize = size * nmemb;
    char* data = (char*)content;
    data[realSize - 1] = 0;

    jsmntok_t tokens[TKN_SIZE];
    const int actual = jsmn_parse(&parser, data, realSize, tokens, TKN_SIZE);
    if (actual < 0) {
        fprintf(stderr, "failed to parse json with error code %d\n", actual);
    }


    return realSize;
}

int main(int argc, char* argv[]) {

    curl_global_init(CURL_GLOBAL_DEFAULT);
    jsmn_init(&parser);

    CURL* curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.coinmarketcap.com/v1/ticker/bitcoin/?convert=EUR");
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_callback);

        CURLcode res = curl_easy_perform(curl);

        if(res != CURLE_OK) {
            fprintf(stderr, "nope : %s\n", curl_easy_strerror(res));
        }

        int status = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
        printf("status code: %d\n", status);

        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}
