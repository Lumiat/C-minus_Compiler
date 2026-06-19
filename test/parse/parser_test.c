#include <stdio.h>
#include <stdlib.h>
#include "../../globals.h"
#include "../../util.h"
#include "../../parse.h"
#include "../../scan.h"

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
    lineno = 0;
    Error = FALSE;
    EchoSource = FALSE;
    TraceScan = FALSE;
    TraceParse = TRUE;
    TraceAnalyze = FALSE;
    TraceCode = FALSE;
    resetScanner();
    TreeNode *t = parse();
    if (TraceParse && t != NULL)
    {
        fprintf(listing, "\nSyntax tree:\n");
        printTree(t);
    }
    fclose(source);
    return 0;
}
