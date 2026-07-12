#pragma once

#include "variable.h"

typedef enum LambdaExprType {
        LAMBDA_NOTHING=0,
        LAMBDA_VARIABLE,
        LAMBDA_ABSTRACTION,
        LAMBDA_APPLICATION,
        LAMBDA_SHORTCUT,
        LAMBDA_ENTRY,
        LAMBDA_NUMERAL
} LambdaExprType;

typedef struct Lambda Lambda;

struct Lambda {
        union {
                struct Variable variable;

                struct {
                        Lambda *body;
                        struct Variable bind;
                } abs;

                struct {
                        Lambda *left;
                        Lambda *right;
                } app;

                char *shortcut;

                struct {
                        Lambda *expression;
                        char *entry;
                } ent;

                int numeral;
        };

        LambdaExprType type;
};

void lambda_free(Lambda *lambda);
int lambda_is_numeral(const Lambda *lambda);
const Lambda *lambda_compare(const Lambda *left, const Lambda *right);