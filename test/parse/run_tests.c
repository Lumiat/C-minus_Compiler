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
#include "../../parse.h"
#include "../../scan.h"
#include "../../util.h"

/* define globals expected by parse/util */
FILE *source;
FILE *listing;
FILE *code;
int lineno = 0;
int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = TRUE;
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
    const char *inputs = "test/parse/inputs";
    const char *expected = "test/parse/expected";
    const char *outputs = "test/parse/outputs";
    const char *listfile = "test/parse/tests.list";

    /* create outputs dir if needed */
    MKDIR(outputs);

    FILE *lst = fopen(listfile, "r");
    if (!lst)
    {
        fprintf(stderr, "Cannot open %s: %s\n", listfile, strerror(errno));
        return 2;
    }

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
        fprintf(stderr, "[PARSE TEST] %s\n", name);
        test_count++;
        char inpath[512], outpath[512], expectpath[512];
        snprintf(inpath, sizeof(inpath), "%s/%s.cm", inputs, name);
        snprintf(outpath, sizeof(outpath), "%s/%s.out", outputs, name);
        snprintf(expectpath, sizeof(expectpath), "%s/%s.expected", expected, name);

        source = fopen(inpath, "r");
        if (!source)
        {
            fprintf(stderr, "Missing input %s\n", inpath);
            any_fail = 1;
            continue;
        }

        listing = fopen(outpath, "w");
        if (!listing)
        {
            fprintf(stderr, "Cannot open output %s\n", outpath);
            fclose(source);
            any_fail = 1;
            continue;
        }

        /* reset scanner state if needed */
        resetScanner();
        fseek(source, 0, SEEK_SET);
        lineno = 0;

        TreeNode *t = parse();
        if (TraceParse && t != NULL)
        {
            // fprintf(listing, "\nC- PARSING: %s-result.txt\n\n", name);
            fprintf(listing, "Syntax tree:\n");
            printTree(t);
        }

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
    }
    fclose(lst);
    fprintf(stderr, "[PARSE TEST] Processed %d tests\n", test_count);
    if (any_fail)
    {
        fprintf(stderr, "[PARSE TEST] Some tests failed\n");
        return 2;
    }
    printf("All parse tests passed.\n");
    return 0;
}
