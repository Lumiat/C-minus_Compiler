#include "../../globals.h"
#include "../../scan.h"
#include "../test_runner.h"

static int runScannerCase(FILE *input, FILE *output, const char *name)
{
    TokenType token;

    (void)name;
    source = input;
    listing = output;
    lineno = 0;
    Error = FALSE;
    EchoSource = TRUE;
    TraceScan = TRUE;
    TraceParse = FALSE;
    TraceAnalyze = FALSE;
    TraceCode = FALSE;
    resetScanner();

    do
    {
        token = getToken();
    } while (token != ENDFILE);

    return 0;
}

int main(void)
{
    return runTestSuite("SCAN", "test/scan", runScannerCase);
}
