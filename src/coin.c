#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "coin.h"
#include "util.h"

#define COLOR_RED "\x1b[31m"
#define COLOR_GREEN "\x1b[32m"
#define COLOR_YELLOW "\x1b[33m"
#define COLOR_RESET "\x1b[0m"
#define COLOR_BYELLOW "\x1b[33;1m"

#define RANK                                                                   \
    { "RANK", 8, 4, 0, 1, 0 }
#define SYMBOL                                                                 \
    { "SYMBOL", 6, 8, 0, 0, 0 }
#define DAY_CHANGE                                                             \
    { "24H (%)", 26, 7, 1, 1, 1 }
#define WEEK_CHANGE                                                            \
    { "7D (%)", 28, 7, 1, 1, 1 }
#define PRICE_USD                                                              \
    { "PRICE (USD)", 10, 12, 0, 1, 1 }
#define PRICE_CON                                                              \
    { "PRICE", 32, 12, 0, 1, 1 }

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
    if (color_enabled) {
        printf(COLOR_BYELLOW);
    }
    for (size_t i = 0; i < size; i++) {
        const char* format = columns[i].right_align ? "%*s " : "%-*s ";
        printf(format, columns[i].padding, columns[i].header);
        if (i < size - 1U) {
            printf(HOR_SEP " ");
        }
    }
    printf("\n");
    for (size_t i = 0; i < size; i++) {
        for (int j = 0; j < columns[i].padding + 1; j++) {
            printf(VER_SEP);
        }
        if (i < size - 1U) {
            printf(CROSS_SEP VER_SEP);
        }
    }
    if (color_enabled) {
        printf(COLOR_RESET);
    }
    printf("\n");
}

static int print_coins(const result_t* res, const column_t columns[],
                       size_t size) {
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
                const char* format =
                    columns[j].right_align ? "%*.*s" : "%-*.*s";
                const char* nformat =
                    columns[j].right_align ? "%*.2f" : "%-*.2f";

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
    char(*urls)[URL_SIZE];
    size_t urlc = 0;
    if (args->specific) {
        urls = malloc(args->specific * URL_SIZE * sizeof(char*));
        while (urlc < args->specific) {
            snprintf(urls[urlc], URL_SIZE,
                     "https://api.coinmarketcap.com/v1/ticker/%s/?convert=%s",
                     args->symbols[urlc], args->convert);
            ++urlc;
        }
    } else {
        urls = malloc(URL_SIZE * sizeof(char*));
        snprintf(urls[0], URL_SIZE,
                 "https://api.coinmarketcap.com/v1/ticker/"
                 "?start=%lu&limit=%lu&convert=%s",
                 args->start, args->limit, args->convert);
        urlc = 1;
    }

    int err = request(urls, urlc, &res);
    free(urls);
    if (err) {
        return err;
    }

    if (!args->convert) {
        column_t columns[] = {RANK, SYMBOL, DAY_CHANGE, WEEK_CHANGE, PRICE_USD};
        err = print_coins(&res, columns, 5);
    } else {
        column_t columns[] = {RANK, SYMBOL, DAY_CHANGE, WEEK_CHANGE, PRICE_CON};
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
        snprintf(url, URL_SIZE,
                 "https://api.coinmarketcap.com/v1/global?convert=%s",
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

    cprintf(COLOR_BYELLOW, "total marketcap (%3s) | ",
            args->convert ? args->convert : "USD");

    printf("%.*s (%.*s%% BTC)\n",
           tokens[cur_offset].end - tokens[cur_offset].start,
           res.data + tokens[cur_offset].start, tokens[6].end - tokens[6].start,
           res.data + tokens[6].start);

    cprintf(COLOR_BYELLOW, "active currencies     | ");

    printf("%.*s\n", tokens[8].end - tokens[8].start,
           res.data + tokens[8].start);

    cprintf(COLOR_BYELLOW, "active markets        | ");

    printf("%.*s\n", tokens[12].end - tokens[12].start,
           res.data + tokens[12].start);

    cprintf(COLOR_BYELLOW, "timestamp             | ");

    time_t time = (time_t)atol(res.data + tokens[14].start);
    struct tm* tm = localtime(&time);
    char date[20];
    strftime(date, sizeof(date), "%b %d %Y %H:%M", tm);
    printf("%s\n", date);

    free(tokens);
    free(res.data);
    return 0;
}

int display_portfolio(const arguments* args) {
    color_enabled = args->color_enabled;
    FILE* fp = fopen(args->portfolio, "re");
    if (!fp) {
        fprintf(stderr, "failed to open %s\n", args->portfolio);
        return -1;
    }

    column_t columns[] = {
        {"", 0, 8, 0, 0, 0},       {"AMOUNT", 0, 14, 0, 1, 0},
        {"PRICE", 0, 10, 0, 1, 0}, {"INVEST", 0, 10, 0, 1, 0},
        {"WORTH", 0, 10, 0, 1, 0}, {"PROFIT", 0, 10, 0, 1, 0},
    };

    char name[64];
    char convert[4];
    double amount;
    double invest;
    double total_invest = 0;
    double total_worth = 0;
    double total_profit = 0;
    if (fscanf(fp, "%3s", convert) != 1) {
        fprintf(stderr, "first line in portfolio should be convert currency\n");
        return -1;
    }
    if (!is_available(convert)) {
        fprintf(stderr, "invalid currency '%s'\n", convert);
        return -1;
    }

    print_header(columns, 6);
    while (fscanf(fp, "%64s %lf %lf", name, &amount, &invest) == 3) {
        char url[URL_SIZE];
        snprintf(url, URL_SIZE,
                 "https://api.coinmarketcap.com/v1/ticker/%s/?convert=%s", name,
                 convert);

        result_t res;
        if (request(&url, 1, &res)) {
            return -1;
        }

        jsmntok_t* tokens;
        parse_json(&res, &tokens);

        cprintf(COLOR_BYELLOW, "%*.*s " HOR_SEP " ", 8,
                tokens[7].end - tokens[7].start, res.data + tokens[7].start);

        size_t convert_offset = convert[0] == 'U' ? 11 : 33;
        double current_price = atof(res.data + tokens[convert_offset].start);

        printf("%*.*f " HOR_SEP " ", 14, 8, amount);
        printf("%*.2f " HOR_SEP " ", 10, current_price);
        printf("%*.2f " HOR_SEP " ", 10, invest);
        total_invest += invest;

        double worth = amount * current_price;
        printf("%*.2f " HOR_SEP " ", 10, worth);
        total_worth += worth;

        double profit = worth - invest;
        if (profit > 0.0) {
            cprintf(COLOR_GREEN, "%*.2f ", 10, profit);
        } else {
            cprintf(COLOR_RED, "%*.2f ", 10, profit);
        }
        total_profit += profit;
        printf("\n");

        free(tokens);
        free(res.data);
    }
    cprintf(COLOR_BYELLOW, "%*s " HOR_SEP " ", columns[0].padding, "TOTAL");
    printf("%*c%*.2f " HOR_SEP " %*.2f " HOR_SEP " ", 30, ' ',
           columns[3].padding, total_invest, columns[4].padding, total_worth);
    if (total_profit > 0.0) {
        cprintf(COLOR_GREEN, "%*.2f", columns[5].padding, total_profit);
    } else {
        cprintf(COLOR_RED, "%*.2f", columns[5].padding, total_profit);
    }
    printf("\n");

    fclose(fp);
    return 0;
}
