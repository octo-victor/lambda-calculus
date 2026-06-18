#pragma once

#include "hashtable.h"

struct Mode {
        bool exit;
        bool interrupt;
        bool reduce;
        bool verbose;
        int  limit;
};

extern struct Mode mode;

void parse_command(char *str, HashTable *table);