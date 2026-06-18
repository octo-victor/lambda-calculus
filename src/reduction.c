#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "alpha_rename.h"
#include "ansi_escapes.h"
#include "commands.h"
#include "duplicate.h"
#include "printing.h"
#include "reduction.h"
#include "stack.h"
#include "variable.h"

#define LONG_CYCLE 10000

static Lambda *get_redex(Lambda *lambda);

static void beta_reduction(Lambda *redex);
static void interrupt_handle(int signal);

bool lambda_normal(Lambda *lambda)
{
        if (lambda == NULL)
                return false;

        Lambda *redex = get_redex(lambda);

        return redex == NULL;
}

Lambda *lambda_reduce(Lambda *lambda)
{
        signal(SIGINT, interrupt_handle);
 
        if (lambda == NULL)
                return NULL;

        bool normal_form;

        if (!mode.reduce) {
                lambda_print(lambda, NULL);

                normal_form = lambda_normal(lambda);

                if (normal_form)
                        printf(ANSI_BLUE " (Normal form.)\n" ANSI_RESET);
                else
                        printf("\n");

                return lambda;
        }

        normal_form = false;

        unsigned int i;

        for (i = 0; i < mode.limit; i++) {
                if (mode.interrupt)
                        return NULL;

                Lambda *redex = get_redex(lambda);

                if (redex == NULL) {
                        normal_form = true;
                        break;
                }

                if (mode.verbose) {
                        printf(ANSI_BLUE "%-5u " ANSI_RESET, i + 1);
                        lambda_print(lambda, redex);
                        printf("\n");
                        
                        // getchar();
                } else if ((i + 1) % LONG_CYCLE == 0) {
                        printf(".\n");
                }

                bool rename = alpha_rename(redex);

                if (!rename)
                        beta_reduction(redex);
        }
        /*

        int numeral = lambda_is_numeral(lambda);

        if (numeral != -1)
                printf("%d", numeral);
        else
                lambda_print(lambda, NULL);
        
        */

        lambda_print(lambda, NULL);

        if (normal_form)
                printf(ANSI_BLUE " (Normal form reached after %d steps.)\n" ANSI_RESET, i);
        else
                printf(ANSI_BLUE " (Normal form not reached.)\n" ANSI_RESET);

        return lambda;
}

void beta_reduction(Lambda *redex)
{
        if (redex == NULL)
                return;

        if (!is_redex(redex))
                return;

        Stack *stack = stack_init();

        if (stack == NULL)
                return;

        Lambda *left = redex->app.left;
        Lambda *argument = redex->app.right;

        Lambda *body = left->abs.body;

        struct Variable bound_var = left->abs.bind;

        free(left);

        Lambda *top = body;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                case LAMBDA_SHORTCUT:
                        // illegal
                        stack_free(stack);
                        return;

                case LAMBDA_VARIABLE:
                        struct Variable var = top->variable;

                        if (!variable_compare(var, bound_var))
                                break;

                        Lambda *dup = lambda_duplicate(argument);

                        *top = *dup;
                        free(dup);

                        break;
                        
                case LAMBDA_ABSTRACTION:
                        struct Variable bind = top->abs.bind;

                        if (variable_compare(bind, bound_var))
                                break;

                        Lambda *body = top->abs.body;
                        stack_push(stack, body);

                        break;

                case LAMBDA_APPLICATION:
                        Lambda *right = top->app.right;
                        Lambda *left = top->app.left;

                        stack_push(stack, right);
                        stack_push(stack, left);

                        break;

                case LAMBDA_NUMERAL:
                        break;
                }

                top = (Lambda *)stack_pop(stack);
        }

        lambda_free(argument);
        *redex = *body;
        free(body);

        stack_free(stack);
}

Lambda *get_redex(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        Stack *stack = stack_init();

        if (stack == NULL)
                return NULL;

        Lambda *top = lambda;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                        Lambda *entry = top->ent.expression;
                        stack_push(stack, entry);

                        break;

                case LAMBDA_SHORTCUT:
                        break;

                case LAMBDA_VARIABLE:
                        break;
                        
                case LAMBDA_ABSTRACTION:
                        Lambda *body = top->abs.body;
                        stack_push(stack, body);

                        break;

                case LAMBDA_APPLICATION:
                        if (is_redex(top)) {
                                stack_free(stack);
                                return top;
                        }

                        Lambda *right = top->app.right;
                        Lambda *left = top->app.left;

                        stack_push(stack, right);
                        stack_push(stack, left);

                        break;

                case LAMBDA_NUMERAL:
                        break;
                }

                top = (Lambda *)stack_pop(stack);
        }

        stack_free(stack);

        return NULL;
}

void interrupt_handle(int signal)
{
        mode.interrupt = true;

        printf(
                ANSI_RED
                "Killed.\n"
                ANSI_RESET
        );
}