#pragma once

#include <stdbool.h>

#define SUBSCRIPT_LIMIT 1000000

struct Variable {
        char letter;
        int subscript;
};

void variable_print(struct Variable variable);

bool variable_compare(struct Variable left, struct Variable right);
bool variable_search(const void *left, const void *right);
