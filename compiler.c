#include "compiler.h"

#include "analyze.h"
#include "parse.h"
#include "scan.h"

static void prepareInput(FILE *input, FILE *output)
{
    source = input;
    listing = output;
    rewind(source);
    clearerr(source);
    resetScanner();
    lineno = 0;
    Error = FALSE;
}

static int scanForErrors(FILE *input, FILE *output)
{
    TokenType current;
    int foundError = FALSE;

    prepareInput(input, output);
    do
    {
        current = getToken();
        if (current == ERROR)
        {
            reportScanError(output);
            foundError = TRUE;
        }
    } while (current != ENDFILE);

    return foundError;
}

CompileStatus compileSource(FILE *input, FILE *output)
{
    TreeNode *syntaxTree;

    EchoSource = FALSE;
    TraceScan = FALSE;
    TraceParse = FALSE;
    TraceAnalyze = FALSE;
    TraceCode = FALSE;

    if (scanForErrors(input, output))
        return COMPILE_LEXICAL_ERROR;

    prepareInput(input, output);
    syntaxTree = parse();
    if (Error)
        return COMPILE_SYNTAX_ERROR;

    Error = FALSE;
    analyze(syntaxTree);
    if (Error)
        return COMPILE_SEMANTIC_ERROR;

    return COMPILE_SUCCESS;
}
