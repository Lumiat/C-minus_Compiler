/*
 * run_tests.c
 * C test runner for the C-Minus scanner.
 * Compiles with: gcc -std=c99 -I. test/run_tests.c SCAN.C UTIL.C -o test/run_tests
 * Usage: test/run_tests
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(a) _mkdir(a)
#else
#include <sys/stat.h>
#define MKDIR(a) mkdir(a, 0755)
#endif

#include "../../globals.h"
#include "../../scan.h"
#include "../../util.h"

/* define globals expected by scanner/util */
FILE *source;
FILE *listing;
FILE *code;
int lineno = 0;
int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = FALSE;
int TraceCode = FALSE;
int Error = FALSE;

static int compare_files(const char *a, const char *b)
{
    FILE *fa = fopen(a, "r");
    if (!fa)
        return -1;
    FILE *fb = fopen(b, "r");
    if (!fb)
    {
        fclose(fa);
        return -1;
    }
    int ok = 1;
    char la[1024];
    char lb[1024];
    while (1)
    {
        char *ra = fgets(la, sizeof(la), fa);
        char *rb = fgets(lb, sizeof(lb), fb);
        if (!ra && !rb)
            break; /* both EOF */
        if (!ra || !rb)
        {
            ok = 0;
            break;
        }
        /* normalize line endings by trimming trailing \r and \n */
        size_t ia = strlen(la);
        while (ia && (la[ia - 1] == '\n' || la[ia - 1] == '\r'))
            la[--ia] = '\0';
        size_t ib = strlen(lb);
        while (ib && (lb[ib - 1] == '\n' || lb[ib - 1] == '\r'))
            lb[--ib] = '\0';
        if (strcmp(la, lb) != 0)
        {
            ok = 0;
            break;
        }
    }
    fclose(fa);
    fclose(fb);
    return ok;
}

int main(int argc, char **argv)
{
    const char *inputs = "test/scan/inputs";
    const char *expected = "test/scan/expected";
    const char *outputs = "test/scan/outputs";
    const char *listfile = "test/scan/tests.list";

    fprintf(stderr, "[DEBUG] Creating output directory...\n");
    fflush(stderr);
    /* create outputs dir if needed */
    MKDIR(outputs);

    fprintf(stderr, "[DEBUG] Opening test list: %s\n", listfile);
    fflush(stderr);
    FILE *lst = fopen(listfile, "r");
    if (!lst)
    {
        fprintf(stderr, "Cannot open %s: %s\n", listfile, strerror(errno));
        return 2;
    }
    fprintf(stderr, "[DEBUG] Test list opened successfully\n");
    fflush(stderr);

    EchoSource = TRUE;
    TraceScan = TRUE;

    char name[256];
    int any_fail = 0;
    int test_count = 0;
    while (fgets(name, sizeof(name), lst))
    {
        /* trim newline */
        size_t ln = strlen(name);
        while (ln && (name[ln - 1] == '\n' || name[ln - 1] == '\r'))
            name[--ln] = '\0';
        if (ln == 0)
            continue;
        fprintf(stderr, "[DEBUG] Processing test: %s\n", name);
        fflush(stderr);
        test_count++;
        char *inpath = NULL, *outpath = NULL, *expectpath = NULL;
        inpath = malloc(strlen(inputs) + 1 + strlen(name) + 4 + 1);
        sprintf(inpath, "%s/%s.cm", inputs, name);
        outpath = malloc(strlen(outputs) + 1 + strlen(name) + 4 + 1);
        sprintf(outpath, "%s/%s.out", outputs, name);
        expectpath = malloc(strlen(expected) + 1 + strlen(name) + 9 + 1);
        sprintf(expectpath, "%s/%s.expected", expected, name);

        fprintf(stderr, "[DEBUG]   Opening input: %s\n", inpath);
        fflush(stderr);
        source = fopen(inpath, "r");
        if (!source)
        {
            fprintf(stderr, "Missing input %s\n", inpath);
            any_fail = 1;
            free(inpath);
            free(outpath);
            free(expectpath);
            continue;
        }
        fprintf(stderr, "[DEBUG]   Input opened, running scanner...\n");
        fflush(stderr);
        listing = fopen(outpath, "w");
        if (!listing)
        {
            fprintf(stderr, "Cannot open output %s\n", outpath);
            fclose(source);
            any_fail = 1;
            free(inpath);
            free(outpath);
            free(expectpath);
            continue;
        }

        resetScanner();
        fseek(source, 0, SEEK_SET);
        lineno = 0;
        TokenType tok;
        do
        {
            tok = getToken();
        } while (tok != ENDFILE);

        fclose(source);
        fclose(listing);

        int ok = compare_files(outpath, expectpath);
        if (ok == 1)
            printf("PASS: %s\n", name);
        else
        {
            printf("FAIL: %s (output: %s, expected: %s)\n", name, outpath, expectpath);
            any_fail = 1;
        }
        free(inpath);
        free(outpath);
        free(expectpath);
    }
    fclose(lst);
    fprintf(stderr, "[DEBUG] Processed %d tests total\n", test_count);
    fflush(stderr);
    if (any_fail)
    {
        fprintf(stderr, "[DEBUG] Some tests failed\n");
        fflush(stderr);
        return 2;
    }
    printf("All tests passed.\n");
    fflush(stdout);
    return 0;
}
