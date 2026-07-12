#pragma once

#include <stdbool.h>

#include "ast.h"

struct ReductionData {
        Lambda *lambda;
        bool normal_form;
        double dt_milli;
        unsigned int steps;
        bool error;
};

bool lambda_normal(Lambda *lambda);
struct ReductionData lambda_reduce(Lambda *lambda);