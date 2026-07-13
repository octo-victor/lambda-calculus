#pragma once

#include "hashtable.h"

#define INFTY -1

typedef enum RedStrategy {
        STRAT_NORMAL,
        STRAT_APPLICATIVE
} RedStrategy;

struct Mode {
        bool exit;
        bool interrupt;
        bool reduce;
        bool verbose;
        int max_steps;
        RedStrategy strat;
        bool simple_print;
};

extern struct Mode mode;

void parse_command(char *str, HashTable *table);