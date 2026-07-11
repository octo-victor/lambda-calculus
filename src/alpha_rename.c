#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "alpha_rename.h"
#include "commands.h"
#include "duplicate.h"
#include "stack.h"

static bool capture_check(Lambda *redex, Stack *right_fv);
static void rename_abstraction(Lambda *abst, Stack *right_fv, Stack *binds);

static Stack *get_inner_binds(Lambda *abst);
static const size_t *push_height(Stack *stack, size_t i);

bool alpha_rename(Lambda *redex)
{
        if (!is_redex(redex) || mode.interrupt)
                return NULL;

        Lambda *right = redex->app.right;

        Stack *right_fv = get_free_variables(right);

        if (right_fv == NULL || stack_peek(right_fv) == NULL) {
                stack_free(right_fv);
                return NULL;
        }

        bool rename = capture_check(redex, right_fv);

        stack_free(right_fv);

        return rename;
}

bool capture_check(Lambda *redex, Stack *right_fv)
{
        Stack *stack = stack_init();
        Stack *height = stack_init();
        Stack *binds = stack_init();

        if (stack == NULL || height == NULL || binds == NULL || mode.interrupt)
                goto error_exit;

        bool rename = false;
        Lambda *top = redex->app.left;

        while (top != NULL) {
                size_t h = stack_height(stack);
                size_t *pop_h = (size_t *)stack_peek(height);

                if (pop_h != NULL && h <= *pop_h) {
                        stack_pop(binds);
                        stack_pop(height);
                        free(pop_h);
                }

                switch (top->type) {
                case LAMBDA_ENTRY:
                        stack_push(stack, top->ent.expression);
                        break;

                case LAMBDA_SHORTCUT:
                        break;

                case LAMBDA_VARIABLE:
                        break;

                case LAMBDA_ABSTRACTION:
                        struct Variable *bind = &top->abs.bind;
                        
                        bool capture = stack_search(right_fv, bind, variable_search);

                        if (capture) {
                                rename_abstraction(top, right_fv, binds);

                                if (mode.interrupt)
                                        goto error_exit;

                                rename = true;
                        }

                        stack_push(stack, top->abs.body);
                        push_height(height, h);
                        stack_push(binds, bind);

                        break;
                
                case LAMBDA_APPLICATION:
                        stack_push(stack, top->app.right);
                        stack_push(stack, top->app.left);
                        
                case LAMBDA_NUMERAL:
                        break;
                }

                top = (Lambda *)stack_pop(stack);

                if (mode.interrupt)
                        goto error_exit;
        }

        size_t *p;

        do {
                p = (size_t *)stack_pop(height);
                free(p);
        } while (p != NULL);
        
        stack_free(stack);
        stack_free(height);
        stack_free(binds);

        return rename;

        error_exit:

        do {
                p = (size_t *)stack_pop(height);
                free(p);
        } while (p != NULL);
        
        stack_free(stack);
        stack_free(height);
        stack_free(binds);

        return false;
}

void rename_abstraction(Lambda *abst, Stack *right_fv, Stack *binds)
{
        if (abst == NULL || abst->type != LAMBDA_ABSTRACTION || mode.interrupt)
                return;

        struct Variable *old_bind = &abst->abs.bind;
        Lambda *body = abst->abs.body;

        Stack *stack = stack_init();
        Stack *inner_binds = get_inner_binds(abst);

        if (stack == NULL || inner_binds == NULL)
                goto error_exit;

        struct Variable new_bind = *old_bind;

        for (int i = -1; i < SUBSCRIPT_LIMIT; i++) {
                if (mode.interrupt)
                        goto error_exit;

                new_bind.subscript = i;

                if (!stack_search(right_fv, &new_bind, variable_search)
                 && !stack_search(inner_binds, &new_bind, variable_search)
                 && !stack_search(binds, &new_bind, variable_search))
                        break;
        }

        stack_free(inner_binds);
        inner_binds = NULL;

        if (new_bind.subscript == SUBSCRIPT_LIMIT) {
                stack_free(stack);
                return;
        }

        Lambda *top = abst->abs.body;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                        break;

                case LAMBDA_SHORTCUT:
                        break;

                case LAMBDA_VARIABLE:
                        if (variable_compare(top->variable, *old_bind))
                                top->variable = new_bind;
                        
                        break;

                case LAMBDA_ABSTRACTION:
                        if (!variable_compare(top->abs.bind, *old_bind))
                                stack_push(stack, top->abs.body);

                        break;

                case LAMBDA_APPLICATION:
                        stack_push(stack, top->app.right);
                        stack_push(stack, top->app.left);

                        break;

                case LAMBDA_NUMERAL:
                        break;
                }

                top = (Lambda *)stack_pop(stack);

                if (mode.interrupt)
                        goto error_exit;
        }

        abst->abs.bind = new_bind;

        stack_free(stack);

        return;

        error_exit:

        stack_free(stack);
        stack_free(inner_binds);

        return;
}

Stack *get_inner_binds(Lambda *abst)
{
        if (abst == NULL || abst->type != LAMBDA_ABSTRACTION || mode.interrupt)
                return NULL;

        struct Variable old_bind = abst->abs.bind;

        Stack *stack = stack_init();
        Stack *inner_binds = stack_init();

        if (stack == NULL || inner_binds == NULL)
                goto error_exit;

        Lambda *top = abst->abs.body;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                        // illegal
                        break;

                case LAMBDA_SHORTCUT:
                        break;

                case LAMBDA_VARIABLE:
                        struct Variable *var = &top->variable;

                        if (!variable_compare(old_bind, *var)
                         && !stack_search(inner_binds, var, variable_search))
                                stack_push(inner_binds, var);

                        break;

                case LAMBDA_ABSTRACTION:
                        struct Variable *bind = &top->abs.bind;

                        if (!variable_compare(old_bind, *bind)
                         && !stack_search(inner_binds, bind, variable_search))
                                stack_push(inner_binds, bind);

                        stack_push(stack, top->abs.body);

                        break;

                case LAMBDA_APPLICATION:
                        stack_push(stack, top->app.right);
                        stack_push(stack, top->app.left);
                        
                        break;

                case LAMBDA_NUMERAL:
                        break;
                }

                top = (Lambda *)stack_pop(stack);

                if (mode.interrupt)
                        goto error_exit;
        }
        
        stack_free(stack);

        return inner_binds;

        error_exit:

        stack_free(stack);
        stack_free(inner_binds);

        return NULL;
}

Stack *get_free_variables(Lambda *lambda)
{
        if (lambda == NULL || mode.interrupt)
                return NULL;

        Stack *stack = stack_init();
        Stack *height = stack_init();

        Stack *free_variables = stack_init();
        Stack *binds = stack_init();

        if (stack == NULL || height == NULL || free_variables == NULL || binds == NULL)
                goto error_exit;
        
        Lambda *top = lambda;

        while (top != NULL) {
                size_t h = stack_height(stack);
                size_t *pop_h = (size_t *)stack_peek(height);

                if (pop_h != NULL && h < *pop_h) {
                        stack_pop(binds);
                        stack_pop(height);
                        free(pop_h);
                }

                switch (top->type) {
                case LAMBDA_ENTRY:
                        stack_push(stack, top->ent.expression);
                        break;

                case LAMBDA_SHORTCUT:
                        break;

                case LAMBDA_VARIABLE:
                        struct Variable *var = &top->variable;

                        if (stack_search(binds, var, variable_search))
                                break;

                        if (!stack_search(free_variables, var, variable_search))
                                stack_push(free_variables, var);
                        
                        break;

                case LAMBDA_ABSTRACTION:
                        struct Variable *bind = &top->abs.bind;

                        stack_push(stack, top->abs.body);

                        stack_push(binds, bind);
                        push_height(height, h);

                        break;

                case LAMBDA_APPLICATION:
                        stack_push(stack, top->app.right);
                        stack_push(stack, top->app.left);
                        
                        break;

                case LAMBDA_NUMERAL:
                        break;
                }

                top = (Lambda *)stack_pop(stack);

                if (mode.interrupt)
                        goto error_exit;
        }

        stack_free(stack);
        
        size_t *p;

        do {
                p = (size_t *)stack_pop(height);
                free(p);
        } while (p != NULL);
        
        stack_free(height);
        stack_free(binds);

        return free_variables;

        error_exit:

        do {
                p = (size_t *)stack_pop(height);
                free(p);
        } while (p != NULL);

        stack_free(stack);
        stack_free(height);
        stack_free(free_variables);
        stack_free(binds);

        return NULL;
}

const size_t *push_height(Stack *stack, size_t height)
{
        if (stack == NULL || mode.interrupt)
                return NULL;

        size_t *mem = malloc(sizeof(*mem));

        *mem = height;

        return stack_push(stack, mem);
}

bool is_redex(Lambda *lambda)
{
        if (lambda == NULL || mode.interrupt)
                return false;

        if (lambda->type != LAMBDA_APPLICATION)
                return false;

        Lambda *left = lambda->app.left;

        if (left == NULL)
                return false;

        return left->type == LAMBDA_ABSTRACTION;
}