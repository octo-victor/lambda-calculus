#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_escapes.h"
#include "parser.h"
#include "hashtable.h"
#include "printing.h"
#include "shortcut.h"
#include "reduction.h"
#include "commands.h"

#define BUF_LEN 65536

void handle(int signal);

const char *hello_message =

"Lambda Calculus (λ-calculus) abstraction and application interpreter.\n"
"Made by octo-victor at "
             ANSI_BLUE "https://github.com/octo-victor/lambda-calculus.\n" ANSI_RESET
"Type \":help\" for more information.\n";

struct Mode mode = {
        .exit = false,
        .interrupt = false,
        .reduce = false,
        .strat = STRAT_NORMAL,
        .verbose = false,
        .limit = 1000
};

int main()
{
        signal(SIGINT, handle);
        
        HashTable *table = hashtable_init();
        char buffer[BUF_LEN];

        printf("%s", hello_message);

        while (!mode.exit) {
                printf("λ> ");

                fgets(buffer, BUF_LEN, stdin);

                if (mode.interrupt)
                        break;
                
                if (buffer[0] == ':') {
                        parse_command(buffer, table);
                        continue;
                }

                Lambda *lambda;
                lambda = lambda_parse(buffer);


                if (lambda == NULL)
                        continue;

                bool valid_term = replace_shortcuts(lambda, table);

                if (!valid_term) {
                        lambda_free(lambda);
                        continue;
                }
                
                lambda = lambda_reduce(lambda);

                if (!hashtable_insert(table, lambda))
                        lambda_free(lambda);

                if (mode.interrupt) {
                        mode.interrupt = false;
                        signal(SIGINT, handle);
                }

                buffer[0] = '\0';
        }

        printf("Exiting.\n");

        hashtable_free(table);

        return 0;
}

void handle(int signal)
{
        mode.interrupt = true;

        printf(
                ANSI_RED
                "Killed.\n"
                ANSI_RESET
        );
}