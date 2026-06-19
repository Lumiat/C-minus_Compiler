#include "globals.h"
#include "util.h"
#include "scan.h"
#include "parse.h"

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    source = fopen(argv[1], "r");
    if (source == NULL)
    {
        fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
        return 1;
    }

    listing = stdout;
    Error = FALSE;
    EchoSource = FALSE;
    TraceScan = FALSE;
    TraceParse = TRUE;
    TraceAnalyze = FALSE;
    TraceCode = FALSE;
    resetScanner();
    lineno = 0;
    TreeNode *syntaxTree = parse();
    fprintf(listing, "Syntax tree:\n");
    printTree(syntaxTree);

    fclose(source);
    return 0;
}
