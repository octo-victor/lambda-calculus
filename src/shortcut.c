#include <stdio.h>
#include <stdlib.h>

#include "ansi_escapes.h"
#include "duplicate.h"
#include "commands.h"
#include "shortcut.h"
#include "stack.h"

static Lambda *generate_numeral(int integer);

bool replace_shortcuts(Lambda *lambda, HashTable *table)
{
        if (lambda == NULL)
                return false;

        Stack *stack = stack_init();

        if (stack == NULL)
                goto error_exit;

        Lambda *top = lambda;

        while (top != NULL) {
                if (mode.interrupt)
                        goto error_exit;

                switch (top->type) {
                case LAMBDA_ENTRY:
                        stack_push(stack, top->ent.expression);
                        break;

                case LAMBDA_SHORTCUT:
                        char *shortcut = top->shortcut;

                        Lambda *entry = hashtable_search(table, shortcut);

                        if (entry == NULL) {
                                printf(
                                        ANSI_RED
                                        "Error. Undefined entry \"%s\".\n"
                                        ANSI_RESET,
                                        shortcut
                                );
                                goto error_exit;
                        }

                        Lambda *duplicate = lambda_duplicate(entry->ent.expression);

                        if (duplicate == NULL) {
                                printf(ANSI_RED "Error. Duplication fail.\n" ANSI_RESET);
                                goto error_exit;
                        }

                        *top = *duplicate;

                        free(shortcut);
                        free(duplicate);                        

                        break;

                case LAMBDA_VARIABLE:
                        break;

                case LAMBDA_ABSTRACTION:
                        stack_push(stack, top->abs.body);
                        break;

                case LAMBDA_APPLICATION:
                        stack_push(stack, top->app.right);
                        stack_push(stack, top->app.left);

                        break;

                case LAMBDA_NUMERAL:
                        int integer = top->numeral;
                        Lambda *numeral = generate_numeral(integer);
                        
                        if (numeral == NULL) {
                                printf(
                                        ANSI_RED
                                        "Error. Failed to generate \"%d\" numeral.\n"
                                        ANSI_RESET,
                                        integer
                                );
                                goto error_exit;
                        }

                        *top = *numeral;
                        free(numeral);
                        
                        break;
                }

                top = (Lambda *)stack_pop(stack);
        }

        stack_free(stack);
        return true;

        error_exit:

        stack_free(stack);
        return false;
}

Lambda *generate_numeral(int integer)
{
        struct Variable var_f = {
                .letter = 'f',
                .subscript = -1
        };

        struct Variable var_x = {
                .letter = 'x',
                .subscript = -1
        };

        if (integer < 0)
                return NULL;

        Lambda *numeral = malloc(sizeof(*numeral));

        if (numeral == NULL)
                return NULL;

        numeral->type = LAMBDA_ABSTRACTION;
        numeral->abs.bind = var_f;
        
        Lambda *inner_abstraction = malloc(sizeof(*inner_abstraction));

        if (inner_abstraction == NULL) {
                free(numeral);
                return NULL;
        }

        inner_abstraction->type = LAMBDA_ABSTRACTION;
        inner_abstraction->abs.bind = var_x;

        numeral->abs.body = inner_abstraction;

        Lambda *right;

        // Particular case without function application 0=\f.\x.x
        if (integer == 0) {
                right = malloc(sizeof(*right));

                inner_abstraction->abs.body = right;

                if (right == NULL) {
                        lambda_free(numeral);
                        return NULL;
                }

                right->type = LAMBDA_VARIABLE;
                right->variable = var_x;

                return numeral;
        }

        // Generate a chain of applications of the form \f.\x.f(f(...(fx)...))
        Lambda *outer_application = malloc(sizeof(*outer_application));
        inner_abstraction->abs.body = outer_application;

        if (outer_application == NULL){
                lambda_free(numeral);
                return NULL;
        }

        Lambda *application = outer_application;

        for (int k = 0; k < integer; k++) {
                if (mode.interrupt) {
                        application->app.left = NULL;
                        application->app.right = NULL;

                        lambda_free(numeral);

                        return NULL;
                }

                application->type = LAMBDA_APPLICATION;

                Lambda *left = malloc(sizeof(*left));
                right = malloc(sizeof(*right));

                application->app.left = left;
                application->app.right = right;

                if (left == NULL || right == NULL) {
                        lambda_free(numeral);
                        return NULL;
                }

                left->type = LAMBDA_VARIABLE;
                left->variable = var_f;

                if (k + 1 == integer) {
                        right->type = LAMBDA_VARIABLE;
                        right->variable = var_x;

                        application->app.left = left;
                        application->app.right = right;
                } else {
                        application->app.left = left;
                        application->app.right = right;

                        application = right;
                }
        }

        return numeral;
}