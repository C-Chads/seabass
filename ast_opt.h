
#ifndef AST_OPT_H
#define AST_OPT_H

/*
    ARENA ALLOCATION OPTIMIZATION FOR FUNCTIONS.
    
    Meant primarily to speed up compiletimes for SEABASS,
    especially compiletime execution.
*/

#include "data.h"


void optimize_fn(symdecl* ss);


#endif


