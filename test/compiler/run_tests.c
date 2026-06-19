#include "../../compiler.h"
#include "../test_runner.h"

static int runCompilerCase(FILE *input, FILE *output, const char *name)
{
    CompileStatus status;

    (void)name;
    status = compileSource(input, output);
    if (status == COMPILE_SUCCESS)
        fprintf(output, "Compilation succeeded\n");

    return 0;
}

int main(void)
{
    return runTestSuite("COMPILER", "test/compiler", runCompilerCase);
}
