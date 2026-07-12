#include <stdio.h>

#include "ast.h"
#include "commands.h"
#include "stack.h"

void lambda_free(Lambda *lambda)
{
        if (lambda == NULL)
                return;

        Stack *stack = stack_init();

        if (stack == NULL) {
                fprintf(stderr, "Fatal error. Unable to initialize stack.\n");
                exit(EXIT_FAILURE);
        }

        Lambda *top = lambda;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                        free(top->ent.entry);
                        stack_push(stack, top->ent.expression);
                        break;
                
                case LAMBDA_SHORTCUT:
                        free(top->shortcut);
                        break;

                case LAMBDA_ABSTRACTION:
                        stack_push(stack, top->abs.body);
                        break;

                case LAMBDA_APPLICATION:
                        stack_push(stack, top->app.right);
                        stack_push(stack, top->app.left);
                        break;
                
                case LAMBDA_NOTHING:
                case LAMBDA_VARIABLE:
                case LAMBDA_NUMERAL:
                        break;
                }

                free(top);
                top = (Lambda *)stack_pop(stack);
        }

        stack_free(stack);
}

int lambda_is_numeral(const Lambda *lambda)
{
        if (lambda == NULL || mode.interrupt)
                return -1;

        if (lambda->type != LAMBDA_ABSTRACTION)
                return -1;

        struct Variable var_f = lambda->abs.bind;

        if (lambda->abs.body->type != LAMBDA_ABSTRACTION)
                return -1;

        struct Variable var_x = lambda->abs.body->abs.bind;

        if (variable_compare(var_f, var_x))
                return -1;

        int numeral = 0;

        Lambda *node = lambda->abs.body->abs.body;

        while (node->type == LAMBDA_APPLICATION) {
                if (mode.interrupt)
                        return -1;

                Lambda *left = node->app.left;
                Lambda *right = node->app.right;

                if (left->type != LAMBDA_VARIABLE
                 || !variable_compare(left->variable, var_f))
                        return -1;

                node = right;
                numeral++;
        }

        if (node->type != LAMBDA_VARIABLE)
                return -1;

        if (variable_compare(node->variable, var_x))
                return numeral;
        else
                return -1;
}

const Lambda *lambda_compare(const Lambda *left, const Lambda *right)
{
        if (left == NULL || right == NULL || mode.interrupt)
                return NULL;

        Stack *left_stack = stack_init();
        Stack *right_stack = stack_init();

        Stack *left_binds = stack_init();
        Stack *right_binds = stack_init();

        if (left_stack == NULL || right_stack == NULL
         || left_binds == NULL || right_binds == NULL) {
                stack_free(left_stack);
                stack_free(right_stack);

                stack_free(left_binds);
                stack_free(right_binds);

                return NULL;
        }

        const Lambda *address = right;

        while (left != NULL && right != NULL) {
                if (left->type != right->type || mode.interrupt) {
                        address = NULL;
                        break;
                }
                
                switch (left->type) {
                case LAMBDA_VARIABLE:
                        size_t left_index = stack_index(left_binds, &left->variable,
                                                        variable_search);
                        size_t right_index = stack_index(right_binds, &right->variable,
                                                        variable_search);

                        if (left_index != right_index) {
                                address = NULL;
                                break;
                        }

                        break;

                case LAMBDA_ABSTRACTION:
                        stack_push(left_stack, left->abs.body);
                        stack_push(right_stack, right->abs.body);

                        stack_push(left_binds, &left->abs.bind);
                        stack_push(right_binds, &right->abs.bind);

                        break;

                case LAMBDA_APPLICATION:
                        stack_push(left_stack, left->app.right);
                        stack_push(left_stack, left->app.left);

                        stack_push(right_stack, right->app.right);
                        stack_push(right_stack, right->app.left);

                        break;

                case LAMBDA_ENTRY:
                        stack_push(left_stack, left->ent.expression);
                        stack_push(right_stack, right->ent.expression);

                        break;
                }

                if (address == NULL)
                        break;

                left = stack_pop(left_stack);
                right = stack_pop(right_stack);
        }

        stack_free(left_stack);
        stack_free(right_stack);

        stack_free(left_binds);
        stack_free(right_binds);

        return address;
}