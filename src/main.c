#include <argp.h>

#include "util.h"
#include "coin.h"

#define SYM_STEP 10

const char* argp_program_bug_address = "https://github.com/Olavhaasie/coinget/issues";
const char* argp_program_version = VERSION;

static struct argp_option options[] = {
        { 0, 0, 0, 0, "Program options:", 1},
        { "start", 's', "NUM", 0, "start displaying from given rank", 0 },
        { "limit", 'l', "NUM", 0, "display NUM cryptos", 0 },
        { "convert", 'c', "SYM", 0, "display value in currency", 0 },
        { "coin-id", 'i', "SYM", 0, "display specific crypto", 0 },
        { "global", 'g', 0, OPTION_ARG_OPTIONAL, "display global crypto information", 0 },
        { "portfolio", 'p', "file", 0, "use given portofolio file", 0},
        { "no-color", 'n', 0, OPTION_ARG_OPTIONAL, "disable color output", 0 },
        { 0, 0, 0, 0, "Informational options:", 0},
        { 0 }
};

static int parse_opt(int key, char* arg, struct argp_state* state) {
    arguments* args = (arguments*)state->input;
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
        case 'p':
            args->portfolio = arg;
            break;
        case 'n':
            args->color_enabled = 0;
            break;
        case 'g':
            args->global = 1;
            break;
        case 'i':
        case ARGP_KEY_ARG:
            if (args->specific == 0) {
                args->symbols = malloc(SYM_STEP * sizeof(char*));
            } else if (args->specific % SYM_STEP == 0) {
                args->symbols = realloc(args->symbols, SYM_STEP * (args->specific + sizeof(char*)));
            }
            args->symbols[args->specific++] = arg;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, "[CRYPTO IDs]",
    "display cryptocurrency values in terminal\v"
    "Every trailing argument will be interpreted as a crypto name.\n"
    "So for example you can chain cryptos:\n"
    "\t$ coinget bitcoin ethereum ripple",
    0, 0, 0 };

int main(int argc, char* argv[]) {
    arguments args;
    args.start = 0;
    args.limit = 25;
    args.convert = NULL;
    args.symbols = NULL;
    args.specific = 0;
    args.global = 0;
    args.portfolio = NULL;
    args.color_enabled = 1;

    argp_parse(&argp, argc, argv, 0, 0, &args);

    int err;
    if (args.global) {
        err = display_global(&args);
    } else if (args.portfolio) {
        err = display_portfolio(args.portfolio);
    } else {
        err = display_result(&args);
    }

    free(args.symbols);
    return err;
}

