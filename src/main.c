#include <argp.h>

#include "coin.h"
#include "util.h"
#include "version.h"

#define SUBCOMMANDS_SIZE 3
#define DEFAULT_LIMIT 25
#define SYM_STEP 10

const char* argp_program_bug_address =
    "https://github.com/Olavhaasie/coinget/issues";
const char* argp_program_version = COINGET_VERSION;

static struct argp_option global_options[] = {
    {0, 0, 0, 0, "global options:", 0},
    {"no-color", 'n', 0, 0, "disable color output", 0},
    {0}};

static struct argp_option list_options[] = {
    {"start", 's', "NUM", 0, "start displaying from given rank", 0},
    {"limit", 'l', "NUM", 0, "display NUM cryptos", 0},
    {"convert", 'c', "SYM", 0, "display value in currency", 0},
    {"coin-id", 'i', "SYM", 0, "display specific crypto", 0},
    {0}};

static struct argp_option stats_options[] = {
    {"convert", 'c', "SYM", 0, "display value in currency", 0}, {0}};

static struct argp_option portfolio_options[] = {
    {"portfolio", 'p', "file", 0, "use given portfolio file", 0}, {0}};

static char list_doc[] = "list ranking and values of cryptos";
static char stats_doc[] = "list global crypto market information";
static char portfolio_doc[] = "list your own investments and profits";

static int list_parse_opt(int key, char* arg, struct argp_state* state) {
    arguments* args = (arguments*)state->input;
    size_t tolong = 0;
    switch (key) {
    case 's':
        tolong = strtoul(arg, NULL, 0);
        if (tolong == 0 || tolong == ULONG_MAX) {
            argp_error(state, "argument must be positive integer");
        } else {
            args->start = tolong - 1;
        }
        break;
    case 'l':
        tolong = strtoul(arg, NULL, 0);
        if (tolong == 0 || tolong == ULONG_MAX) {
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
    case ARGP_KEY_ARG:
        if (args->specific == 0) {
            args->symbols = malloc(SYM_STEP * sizeof(char*));
        } else if (args->specific % SYM_STEP == 0) {
            args->symbols = realloc(
                args->symbols, SYM_STEP * (args->specific + sizeof(char*)));
        }
        args->symbols[args->specific++] = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static int stats_parse_opt(int key, char* arg, struct argp_state* state) {
    arguments* args = (arguments*)state->input;
    switch (key) {
    case 'c':
        if (is_available(arg)) {
            args->convert = arg;
        } else {
            argp_error(state, "invalid currency '%s'", arg);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static int portfolio_parse_opt(int key, char* arg, struct argp_state* state) {
    arguments* args = (arguments*)state->input;
    switch (key) {
    case 'p':
    case ARGP_KEY_ARG:
        args->portfolio = arg;
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp list_argp = {
    list_options, list_parse_opt, 0, list_doc, 0, 0, 0};
static struct argp stats_argp = {
    stats_options, stats_parse_opt, 0, stats_doc, 0, 0, 0};
static struct argp portfolio_argp = {portfolio_options,
                                     portfolio_parse_opt,
                                     "[PORTFOLIO]",
                                     portfolio_doc,
                                     0,
                                     0,
                                     0};

typedef int subcommand(const arguments*);

static struct subcommand_t {
    char* name;
    struct argp* argp;
    subcommand* cmd;
} subcommands[SUBCOMMANDS_SIZE] = {
    {"list", &list_argp, display_result},
    {"stats", &stats_argp, display_global},
    {"portfolio", &portfolio_argp, display_portfolio}};
static int selected_subcommand = -1;

void parse_subcommand(struct argp_state* state, const char* name,
                      struct argp* argp) {
    int argc = state->argc - state->next + 1;
    char** argv = &state->argv[state->next - 1];
    char* arg0 = argv[0];

    argv[0] = malloc(strlen(state->name) + strlen(name) + 2);
    if (!argv[0]) {
        argp_failure(state, 1, ENOMEM, 0);
    }

    sprintf(argv[0], "%s %s", state->name, name);

    argp_parse(argp, argc, argv, ARGP_IN_ORDER, &argc, state->input);

    free(argv[0]);
    argv[0] = arg0;

    state->next += argc - 1;
}

static int global_parse_opt(int key, char* arg, struct argp_state* state) {
    arguments* args = (arguments*)state->input;
    switch (key) {
    case 'n':
        args->color_enabled = 0;
        break;
    case ARGP_KEY_ARG:
        for (size_t i = 0; i < SUBCOMMANDS_SIZE; i++) {
            if (strncmp(arg, subcommands[i].name,
                        strlen(subcommands[i].name)) == 0) {
                parse_subcommand(state, arg, subcommands[i].argp);
                selected_subcommand = i;
                break;
            }
        }
        if (selected_subcommand == -1) {
            argp_error(state, "%s is not a command", arg);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp global_argp = {
    global_options,
    global_parse_opt,
    "COMMAND [OPTION...]",
    "display cryptocurrency values in terminal\v"
    " commands:\n"
    "      list        shows the ranking and values of all cryptos\n"
    "      stats       shows global information of the crypto market\n"
    "      portfolio   shows your own portfolio of investments\n",
    0,
    0,
    0};

int main(int argc, char* argv[]) {
    arguments args;
    args.start = 0;
    args.limit = DEFAULT_LIMIT;
    args.convert = NULL;
    args.symbols = NULL;
    args.specific = 0;
    args.global = 0;
    args.portfolio = getenv("HOME");
    strcat(args.portfolio, "/.coins");
    args.color_enabled = 1;

    if (argc < 2) {
        argp_help(&global_argp, stdout, ARGP_HELP_SEE, "coinget");
        return 0;
    }

    argp_parse(&global_argp, argc, argv, ARGP_IN_ORDER, 0, &args);

    int err = subcommands[selected_subcommand].cmd(&args);
    free(args.symbols);
    return err;
}
