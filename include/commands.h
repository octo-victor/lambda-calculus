#pragma once

#include "hashtable.h"

typedef enum RedStrategy {
        STRAT_NORMAL,
        STRAT_EAGER
} RedStrategy;

struct Mode {
        bool exit;
        bool interrupt;
        bool reduce;
        bool verbose;
        int limit;
        RedStrategy strat;
        bool simple_print;
};

extern struct Mode mode;

void parse_command(char *str, HashTable *table);