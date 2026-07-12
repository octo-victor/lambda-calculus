#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alpha_rename.h"
#include "ansi_escapes.h"
#include "clock.h"
#include "commands.h"
#include "duplicate.h"
#include "hashtable.h"
#include "printing.h"
#include "reduction.h"
#include "stack.h"
#include "variable.h"

#define LONG_CYCLE 10000

static Lambda *get_redex(Lambda *lambda);
static Lambda *get_redex_normal(Lambda *lambda);
static Lambda *get_redex_eager(Lambda *lambda);

static void beta_reduction(Lambda *redex);

bool lambda_normal(Lambda *lambda)
{
        if (lambda == NULL)
                return false;

        Lambda *redex = get_redex(lambda);

        return redex == NULL;
}

struct ReductionData lambda_reduce(Lambda *lambda)
{
        if (lambda == NULL)
                goto error_exit;

        clock_begin();

        unsigned int i;
        bool normal_form = false;

        for (i = 0; i < mode.limit; i++) {
                Lambda *redex = get_redex(lambda);

                if (redex == NULL) {
                        normal_form = true;
                        break;
                }

                if (mode.verbose) {
                        printf(ANSI_BLUE "%-5u " ANSI_RESET, i + 1);
                        lambda_print(NULL, lambda, redex, false);
                        printf("\n");
                } else if ((i + 1) % LONG_CYCLE == 0) {
                        printf("%d/%d\n", i+1, mode.limit);
                }

                bool rename = alpha_rename(redex);

                if (!rename)
                        beta_reduction(redex);
                
                if (mode.interrupt)
                        goto error_exit;
        }

        clock_end();

        double dt_milli = get_dt() * 1e3;

        struct ReductionData data = {
                .lambda = lambda,
                .dt_milli = dt_milli,
                .normal_form = normal_form,
                .steps = i,
                .error = false
        };

        return data;

        error_exit:

        return (struct ReductionData){
                .lambda = NULL,
                .normal_form = false,
                .dt_milli = 0.0,
                .steps = 0,
                .error = true
        };
}

void beta_reduction(Lambda *redex)
{
        if (redex == NULL || mode.interrupt)
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
        redex->type = LAMBDA_NOTHING;

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

                        if (dup == NULL || mode.interrupt) {
                                lambda_free(dup);
                                goto error_exit;
                        }

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

                if (mode.interrupt)
                        goto error_exit;
        }

        lambda_free(argument);
        *redex = *body;
        free(body);

        stack_free(stack);

        return;

        error_exit:

        lambda_free(body);
        lambda_free(argument);
        
        stack_free(stack);
}

Lambda *get_redex(Lambda *lambda)
{
        switch (mode.strat) {
        case STRAT_NORMAL:
                return get_redex_normal(lambda);
                break;
        
        case STRAT_EAGER:
                return get_redex_eager(lambda);
                break;
        }

        return NULL;
}

Lambda *get_redex_normal(Lambda *lambda)
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

                if (mode.interrupt) {
                        stack_free(stack);
                        return NULL;
                }
        }

        stack_free(stack);

        return NULL;
}

Lambda *get_redex_eager(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        Stack *stack = stack_init();

        if (stack == NULL)
                return NULL;

        Lambda *redex = NULL;
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
                                redex = top;
                                stack_clear(stack);
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
                
                if (mode.interrupt) {
                        stack_free(stack);
                        return NULL;
                }
        }

        stack_free(stack);

        return redex;
}