#include <string.h>

#include "stack.h"

#define STACK_CAP 32

struct Stack {
        const void **array;
        size_t top;
        size_t capacity;
};

Stack *stack_init()
{
        Stack *stack = malloc(sizeof(*stack));
        const void **array = malloc(sizeof(*stack->array) * STACK_CAP);

        if (stack == NULL || array == NULL) {
                free(stack);
                free(array);
                return NULL;
        }

        stack->array = array;
        stack->top = 0;
        stack->capacity = STACK_CAP;

        return stack;
}

void stack_free(Stack *stack)
{
        if (stack == NULL)
                return;

        free(stack->array);
        free(stack);
}

const void *stack_push(Stack *stack, const void *address)
{
        if (stack == NULL || address == NULL)
                return NULL;

        if (stack->top + 1 == stack->capacity) {
                const void **array = realloc(stack->array, sizeof(*array) * (stack->capacity << 1));

                if (array == NULL)
                        return NULL;
                
                stack->array = array;
                stack->capacity <<= 1;
        }

        stack->array[stack->top++] = address;

        return address;
}

const void *stack_pop(Stack *stack)
{
        if (stack == NULL || stack->top == 0)
                return NULL;

        return stack->array[--stack->top];
}

const void *stack_peek(Stack *stack)
{
        if (stack == NULL || stack->top == 0)
                return NULL;

        return stack->array[stack->top - 1];
}

const void *stack_search(Stack *stack, const void *address,
                         bool compare(const void *left, const void *right))
{
        if (stack == NULL || address == NULL)
                return NULL;

        for (size_t i = stack->top; i > 0; i--) {
                const void *entry = stack->array[i - 1];

                if (compare(address, entry))
                        return entry;
        }

        return NULL;
}

size_t stack_index(Stack *stack, const void *address,
                   bool compare(const void *left, const void *right))
{
        if (stack == NULL || address == NULL)
                return 0;

        for (size_t i = stack->top; i > 0; i--) {
                const void *entry = stack->array[i - 1];

                if (compare(address, entry))
                        return i;
        }

        return 0;
}

void stack_clear(Stack *stack)
{
        if (stack != NULL)
                stack->top = 0;
}

size_t stack_height(Stack *stack)
{
        if (stack == NULL)
                return 0;

        return stack->top + 1;
}