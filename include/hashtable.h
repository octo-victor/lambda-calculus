#pragma once

#include "ast.h"

typedef struct HashTable HashTable;

HashTable *hashtable_init();
void hashtable_free(HashTable *table);

Lambda *hashtable_insert(HashTable *table, Lambda *lambda);
Lambda *hashtable_search(HashTable *table, const char *key);
Lambda *hashtable_delete(HashTable *table, const char *key);

void hashtable_print(HashTable *table);
bool lambda_print_simple(const HashTable *table, const Lambda *lambda);