#ifndef _COMPILER_H_
#define _COMPILER_H_

#include "globals.h"

typedef enum
{
    COMPILE_SUCCESS,
    COMPILE_LEXICAL_ERROR,
    COMPILE_SYNTAX_ERROR,
    COMPILE_SEMANTIC_ERROR
} CompileStatus;

CompileStatus compileSource(FILE *input, FILE *output);

#endif
