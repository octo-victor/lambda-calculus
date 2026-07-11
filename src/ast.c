#include <stdio.h>

#include "ast.h"
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

int lambda_is_numeral(Lambda *lambda)
{
        if (lambda == NULL)
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