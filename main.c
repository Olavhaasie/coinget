#include <stdio.h>
#include <argp.h>

#include "coin.h"

const char *argp_program_bug_address = "https://github.com/Olavhaasie/coinget/issues";
const char *argp_program_version = "coinget v1.0";

struct argp_option options[] = {
        { "start", 's', "NUM", 0, "start displaying from given rank"},
        { "limit", 'l', "NUM", 0, "display NUM cryptos"},
        { "convert", 'c', "SYM", 0, "display value in currency"},
        { 0 }
};

typedef struct {
    size_t start;
    size_t limit;
    char* convert;
} arguments;

static int parse_opt (int key, char* arg, struct argp_state* state) {
    arguments* args = state->input;
    switch (key) {
        case 's':
            args->start = atoi(arg) - 1;
            break;
        case 'l':
            args->limit = atoi(arg);
            break;
        case 'c':
            args->convert = arg;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

int main(int argc, char* argv[]) {
    struct argp argp = { options, parse_opt, 0, "display cryptocurrency values in terminal" };
    arguments args;
    args.start = 0;
    args.limit = 25;
    args.convert = "EUR";
    argp_parse(&argp, argc, argv, 0, 0, &args);

    show_coins(args.start, args.limit, args.convert);

    return 0;
}


