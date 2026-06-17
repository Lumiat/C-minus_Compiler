/*
 * scanner_test.c
 * Simple test harness for the C-Minus scanner (uses existing scanner implementation)
 * Usage: compile together with the project's SCAN.C and UTIL.C, then run:
 *   ./scanner_test <input-file>
 * It prints tokens using the project's `printToken` formatting to stdout.
 */

#include <stdio.h>
#include <stdlib.h>
#include "../../globals.h"
#include "../../scan.h"
#include "../../util.h"

/* define globals expected by scanner/util */
FILE *source;
FILE *listing;
FILE *code;
int lineno = 0;
int EchoSource = TRUE;
int TraceScan = TRUE;
int TraceParse = FALSE;
int TraceAnalyze = FALSE;
int TraceCode = FALSE;
int Error = FALSE;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <input-file>\n", argv[0]);
        return 2;
    }
    const char *infile = argv[1];
    source = fopen(infile, "r");
    if (!source)
    {
        perror("fopen");
        return 2;
    }
    listing = stdout; /* print results to stdout so test runner can capture them */
    lineno = 0;
    resetScanner();

    TokenType tok;
    do
    {
        tok = getToken();
    } while (tok != ENDFILE);

    fclose(source);
    return 0;
}
