#include "../../analyze.h"
#include "../../globals.h"
#include "../../parse.h"
#include "../../scan.h"
#include "../test_runner.h"

static int hasLexicalErrors(FILE *input)
{
    TokenType token;
    int foundError = FALSE;

    source = input;
    rewind(source);
    clearerr(source);
    resetScanner();
    lineno = 0;
    TraceScan = FALSE;

    do
    {
        token = getToken();
        if (token == ERROR)
            foundError = TRUE;
    } while (token != ENDFILE);

    return foundError;
}

static int runAnalyzerCase(FILE *input, FILE *output, const char *name)
{
    FILE *diagnosticSink;
    TreeNode *syntaxTree;

    if (hasLexicalErrors(input))
    {
        fprintf(stderr,
                "[ANALYZE TEST] fixture precondition failed: %s contains lexical errors\n",
                name);
        return 1;
    }

    diagnosticSink = tmpfile();
    if (diagnosticSink == NULL)
    {
        fprintf(stderr, "[ANALYZE TEST] cannot create diagnostic sink for %s\n", name);
        return 1;
    }

    source = input;
    listing = diagnosticSink;
    rewind(source);
    clearerr(source);
    resetScanner();
    lineno = 0;
    Error = FALSE;
    EchoSource = FALSE;
    TraceScan = FALSE;
    TraceParse = FALSE;
    TraceAnalyze = FALSE;
    TraceCode = FALSE;

    syntaxTree = parse();
    if (Error)
    {
        fprintf(stderr,
                "[ANALYZE TEST] fixture precondition failed: %s contains syntax errors\n",
                name);
        fclose(diagnosticSink);
        return 1;
    }
    fclose(diagnosticSink);

    listing = output;
    Error = FALSE;
    analyze(syntaxTree);
    return 0;
}

int main(void)
{
    return runTestSuite("ANALYZE", "test/analyze", runAnalyzerCase);
}
