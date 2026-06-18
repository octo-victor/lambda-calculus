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

const char *hello_message =

"Lambda Calculus (λ-calculus) abstraction and application interpreter.\n"
"Made by victorsavas at "
             ANSI_BLUE "https://github.com/victorsavas/lambda-calculus.\n" ANSI_RESET
"Type \":help\" for more information.\n";

struct Mode mode = {
        .exit = false,
        .interrupt = false,
        .reduce = false,
        .verbose = false,
        .limit = 1000
};

HashTable *table;

static void interrupt_handle(int signal);

int main()
{
        table = hashtable_init();
        char buffer[BUF_LEN];

        printf("%s", hello_message);

        while (!mode.exit) {
                signal(SIGINT, interrupt_handle);

                printf("λ> ");

                fgets(buffer, BUF_LEN, stdin);
                
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
                
                lambda_reduce(lambda);

                mode.interrupt = false;

                if (!hashtable_insert(table, lambda))
                        lambda_free(lambda);
        }

        printf("Exiting.\n");

        hashtable_free(table);

        return 0;
}

void interrupt_handle(int signal)
{
        printf("\nExiting.\n");
        hashtable_free(table);

        exit(1);
}