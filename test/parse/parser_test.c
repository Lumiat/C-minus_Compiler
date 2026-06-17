#include <stdio.h>
#include <stdlib.h>
#include "../../globals.h"
#include "../../util.h"
#include "../../parse.h"

FILE *source;
FILE *listing;
FILE *code;
int lineno = 1;
int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = TRUE;
int TraceAnalyze = FALSE;
int TraceCode = FALSE;
int Error = FALSE;

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <source-file>\n", argv[0]);
        return 1;
    }
    source = fopen(argv[1], "r");
    if (!source)
    {
        perror("fopen source");
        return 1;
    }
    listing = stdout;
    TreeNode *t = parse();
    if (TraceParse && t != NULL)
    {
        fprintf(listing, "\nSyntax tree:\n");
        printTree(t);
    }
    fclose(source);
    return 0;
}
