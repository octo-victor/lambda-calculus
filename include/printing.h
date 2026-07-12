#pragma once

#include "ast.h"
#include "hashtable.h"

void lambda_print(const HashTable *table, const Lambda *lambda,
                  const Lambda *redex, bool simplify);