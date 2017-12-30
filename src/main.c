#include <stdio.h>
#include <argp.h>

#include "coin.h"

const char* argp_program_bug_address = "https://github.com/Olavhaasie/coinget/issues";
const char* argp_program_version = VERSION;

static struct argp_option options[] = {
        { 0, 0, 0, 0, "Program options:", 1},
        { "start", 's', "NUM", 0, "start displaying from given rank", 0 },
        { "limit", 'l', "NUM", 0, "display NUM cryptos", 0 },
        { "convert", 'c', "SYM", 0, "display value in currency", 0 },
        { "coin-id", 'i', "SYM", 0, "display specific crypto", 0 },
        { "no-color", 'n', 0, 0, "disable color output", 0 },
        { 0, 0, 0, 0, "Informational options:", 0},
        { 0 }
};

static int parse_opt (int key, char* arg, struct argp_state* state) {
    arguments* args = state->input;
    switch (key) {
        case 's':
            {
                const long tolong = atol(arg);
                if (tolong <= 0) {
                    return ARGP_ERR_UNKNOWN;
                } else {
                    args->start = tolong - 1;
                }
                break;
            }
        case 'l':
            {
                const long tolong = atol(arg);
                if (tolong <= 0) {
                    return ARGP_ERR_UNKNOWN;
                } else {
                    args->limit = tolong;
                }
                break;
            }
        case 'c':
            args->convert = arg;
            break;
        case 'i':
            args->symbol = arg;
            args->specific = 1;
            break;
        case 'n':
            args->color_enabled = 0;
            break;
        case ARGP_KEY_ARG:
            if (args->specific == 0) {
                args->symbol = arg;
                args->specific = 1;
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, "[CRYPTO ID]", "display cryptocurrency values in terminal" , 0, 0, 0 };

int main(int argc, char* argv[]) {
    arguments args;
    args.start = 0;
    args.limit = 25;
    args.convert = "EUR";
    args.symbol = "";
    args.specific = 0;
    args.color_enabled = 1;

    argp_parse(&argp, argc, argv, 0, 0, &args);

    coin_init();

    return display_result(&args);
}

