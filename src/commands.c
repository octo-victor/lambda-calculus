#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_escapes.h"
#include "commands.h"

static void command_remove(HashTable *table);
static void max_steps();

const char *help_message =

"Lambda Calculus (λ-calculus) abstraction and application interpreter.\n"
"Made by victorsavas at "
             ANSI_BLUE "https://github.com/octo-victor/lambda-calculus"
                                                          ANSI_RESET ".\n"
"\n"
"Command list:\n"
"  :applicative                         Enables reduction with applicative strategy (innermost-leftmost)\n"
"  :clear                               Clears the screen\n"
"  :entries                             Prints out all the shortcuts stored\n"
"  :exit                                Terminates the program\n"
"  :help                                Prints this message\n"
"  :maxsteps <steps>                    Sets reduction limit\n"
"  :noreduction                         Disables reduction (on by default)\n"
"  :normal                              Enables reduction with normal strategy (outmost-leftmost)\n"
"  :noverbose                           Disables step-by-step printing\n"
"  :remove <entry>                      Deletes one entry from the hashtable\n"
"  :simple                              Simplifies output printing\n"
"  :sparse                              Prints the full syntax of the output\n"
"  :verbose                             Prints reduction steps\n"
"\n"
"Syntax:\n"
"  expression -> variable\n"
"              | abstraction\n"
"              | application\n"
"  variable -> a-z [0-9]*\n"                
"  abstraction -> \\ variable . expression\n"
"  application -> expression expression\n"
"\n"
"To resolve the ambiguity of this syntax, parenthesis are used.\n"
"If parenthesis are ommited, the following rules apply:\n"
"  1. Application has higher precedence than abstraction;\n"
"  2. Application is left-associative (MNP is equivalent to (MN)P);\n"
"  3. Abstraction is right-associative (\\x.\\y.M is equivalent to \\x.(\\y.M)).\n"
"\n"
"Particularly, these rules entail that an abstraction extends as far to the right "
"as possible.\n"
"\n"
"Within this interpreter, it is possible to create shortcuts to expressions.\n"
"  shortcut -> identifier = expression\n"
"  identifier -> [A-Z] [A-Z_]*\n"
"\n"
"For convenience, the interpreter is initialized with some relevant shortcuts.\n"
"To see them, use the command \":entries\".\n"
"It is possible to overwrite an entries and to remove them using the \":remove\" command.\n"
"\n"
"Furthermore, for convenience, the interpreter also parses integers as Church numerals.\n"
"For example, typing \"2\" produces the corresponding Church numeral \"\\f.\\x.f(fx)\".\n"
"\n";

const char space[] = " \t\n\v\f\r";

void parse_command(char *str, HashTable *table)
{
        if (str == NULL)
                return;

        char *token = strtok(str, space);

        if (strcmp(token, ":exit") == 0) {
                mode.exit = true;
        } else if (strcmp(token, ":help") == 0) {
                printf("%s", help_message);
        } else if (strcmp(token, ":entries") == 0) {
                hashtable_print(table);
        } else if (strcmp(token, ":remove") == 0) {
                command_remove(table);
        } else if (strcmp(token, ":normal") == 0) {
                mode.reduce = true;
                mode.strat = STRAT_NORMAL;
                if (mode.max_steps == 0)
                        mode.max_steps = 1000;
        } else if (strcmp(token, ":applicative") == 0) {
                mode.reduce = true;
                mode.strat = STRAT_APPLICATIVE;
                if (mode.max_steps == 0)
                        mode.max_steps = 1000;
        }
        else if (strcmp(token, ":noreduction") == 0) {
                mode.reduce = false;
        } else if (strcmp(token, ":verbose") == 0) {
                mode.verbose = true;
        } else if (strcmp(token, ":noverbose") == 0) {
                mode.verbose = false;
        } else if (strcmp(token, ":maxsteps") == 0) {
                max_steps();
        } else if (strcmp(token, ":clear") == 0) {
                printf(ANSI_CLEAR);
        } else if (strcmp(token, ":simple") == 0) {
                mode.simple_print = true;
        } else if (strcmp(token, ":sparse") == 0) {
                mode.simple_print = false;
        } else {
                printf(
                        ANSI_RED
                        "Invalid command \"%s\".\n"
                        "For more information, type \":help\".\n"
                        ANSI_RESET,
                        token
                );
        }
}

void command_remove(HashTable *table)
{
        char *token = strtok(NULL, space);

        if (token == NULL) {
                printf(":remove <entry>\n");
                return;
        }

        bool success = hashtable_delete(table, token);

        if (success)
                printf("Entry \"%s\" removed successfully.\n", token);
        else
                printf("No \"%s\" entry found.\n", token);
}

void max_steps()
{
        char *token = strtok(NULL, space);

        if (token == NULL) {
                printf(
                        ANSI_RED
                        "Syntax error. Expected argument <steps> (integer or \"none\").\n"
                        ANSI_RESET
                        ":maxsteps <steps>\n"
                );
                return;
        }

        if (strcmp(token, "none") == 0) {
                printf(
                        "Warning: some expressions may never terminate, even on normal mode.\n"
                        "Press CTRL+C to abort reduction.\n"
                );

                mode.max_steps = INFTY;

                return;
        }

        if (!isdigit(token[0])) {
                printf(
                        ANSI_RED
                        "Syntax error. Argument <steps> must be integer or \"none\".\n"
                        ANSI_RESET
                        ":maxsteps <steps>\n"
                );
                return;
        }

        int limit = atoi(token);

        if (limit == 0) {
                mode.reduce = false;
                mode.max_steps = limit;

                return;
        }

        mode.reduce = true;
        mode.max_steps = limit;
}