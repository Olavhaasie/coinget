#include <stdio.h>
#include <argp.h>
#include <string.h>

#include "coin.h"

#define CURR_SIZE 31

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

static const char* currencies[] = {
    "AUD", "BRL", "CAD", "CHF", "CLP", "CNY", "CZK", "DKK",
    "EUR", "GBP", "HKD", "HUF", "IDR", "ILS", "INR", "JPY",
    "KRW", "MXN", "MYR", "NOK", "NZD", "PHP", "PKR", "PLN",
    "RUB", "SEK", "SGD", "THB", "TRY", "TWD", "ZAR" };

static int is_available(const char* curr) {
    if (strlen(curr) == 3) {
        for (size_t i = 0; i < CURR_SIZE; i++) {
            if (strncmp(curr, currencies[i], 3) == 0) {
                return 1;
            }
        }
    }
    return 0;
}

static int parse_opt (int key, char* arg, struct argp_state* state) {
    arguments* args = state->input;
    long tolong = 0;
    switch (key) {
        case 's':
            tolong = atol(arg);
            if (tolong <= 0) {
                argp_error(state, "argument must be positive integer");
            } else {
                args->start = tolong - 1;
            }
            break;
        case 'l':
            tolong = atol(arg);
            if (tolong <= 0) {
                argp_error(state, "argument must be positive integer");
            } else {
                args->limit = tolong;
            }
            break;
        case 'c':
            if (is_available(arg)) {
                args->convert = arg;
            } else {
                argp_error(state, "invalid currency '%s'", arg);
            }
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

static struct argp argp = { options, parse_opt, "[CRYPTO ID]", "display cryptocurrency values in terminal", 0, 0, 0 };

int main(int argc, char* argv[]) {
    arguments args;
    args.start = 0;
    args.limit = 25;
    args.convert = "";
    args.symbol = "";
    args.specific = 0;
    args.color_enabled = 1;

    argp_parse(&argp, argc, argv, 0, 0, &args);

    coin_init();

    return display_result(&args);
}

