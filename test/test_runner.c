#include "test_runner.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../globals.h"

#ifdef _WIN32
#include <direct.h>
#define MAKE_DIRECTORY(path) _mkdir(path)
#else
#include <sys/stat.h>
#define MAKE_DIRECTORY(path) mkdir(path, 0755)
#endif

static int compareFiles(const char *actualPath, const char *expectedPath)
{
    FILE *actual = fopen(actualPath, "r");
    FILE *expected;
    char actualLine[1024];
    char expectedLine[1024];
    int equal = TRUE;

    if (actual == NULL)
        return FALSE;

    expected = fopen(expectedPath, "r");
    if (expected == NULL)
    {
        fclose(actual);
        return FALSE;
    }

    while (TRUE)
    {
        char *actualResult = fgets(actualLine, sizeof(actualLine), actual);
        char *expectedResult = fgets(expectedLine, sizeof(expectedLine), expected);
        size_t actualLength;
        size_t expectedLength;

        if (actualResult == NULL && expectedResult == NULL)
            break;
        if (actualResult == NULL || expectedResult == NULL)
        {
            equal = FALSE;
            break;
        }

        actualLength = strlen(actualLine);
        while (actualLength > 0 &&
               (actualLine[actualLength - 1] == '\n' ||
                actualLine[actualLength - 1] == '\r'))
            actualLine[--actualLength] = '\0';

        expectedLength = strlen(expectedLine);
        while (expectedLength > 0 &&
               (expectedLine[expectedLength - 1] == '\n' ||
                expectedLine[expectedLength - 1] == '\r'))
            expectedLine[--expectedLength] = '\0';

        if (strcmp(actualLine, expectedLine) != 0)
        {
            equal = FALSE;
            break;
        }
    }

    fclose(actual);
    fclose(expected);
    return equal;
}

static char *buildPath(const char *directory, const char *name, const char *suffix)
{
    size_t length = strlen(directory) + 1 + strlen(name) + strlen(suffix) + 1;
    char *path = (char *)malloc(length);

    if (path != NULL)
        snprintf(path, length, "%s/%s%s", directory, name, suffix);
    return path;
}

int runTestSuite(const char *suiteLabel, const char *baseDirectory,
                 TestCaseFunction runCase)
{
    char inputDirectory[512];
    char expectedDirectory[512];
    char outputDirectory[512];
    char listPath[512];
    FILE *list;
    char name[256];
    int failed = FALSE;
    int testCount = 0;

    snprintf(inputDirectory, sizeof(inputDirectory), "%s/inputs", baseDirectory);
    snprintf(expectedDirectory, sizeof(expectedDirectory), "%s/expected", baseDirectory);
    snprintf(outputDirectory, sizeof(outputDirectory), "%s/outputs", baseDirectory);
    snprintf(listPath, sizeof(listPath), "%s/tests.list", baseDirectory);

    if (MAKE_DIRECTORY(outputDirectory) != 0 && errno != EEXIST)
    {
        fprintf(stderr, "Cannot create %s: %s\n", outputDirectory, strerror(errno));
        return 2;
    }

    list = fopen(listPath, "r");
    if (list == NULL)
    {
        fprintf(stderr, "Cannot open %s: %s\n", listPath, strerror(errno));
        return 2;
    }

    while (fgets(name, sizeof(name), list) != NULL)
    {
        char *inputPath;
        char *expectedPath;
        char *outputPath;
        FILE *input;
        FILE *output;
        size_t length = strlen(name);
        int caseResult;

        while (length > 0 &&
               (name[length - 1] == '\n' || name[length - 1] == '\r'))
            name[--length] = '\0';
        if (length == 0)
            continue;

        testCount++;
        fprintf(stderr, "[%s TEST] %s\n", suiteLabel, name);

        inputPath = buildPath(inputDirectory, name, ".cm");
        expectedPath = buildPath(expectedDirectory, name, ".expected");
        outputPath = buildPath(outputDirectory, name, ".out");
        if (inputPath == NULL || expectedPath == NULL || outputPath == NULL)
        {
            fprintf(stderr, "Out of memory while preparing test %s\n", name);
            free(inputPath);
            free(expectedPath);
            free(outputPath);
            failed = TRUE;
            continue;
        }

        input = fopen(inputPath, "r");
        if (input == NULL)
        {
            fprintf(stderr, "Missing input %s\n", inputPath);
            free(inputPath);
            free(expectedPath);
            free(outputPath);
            failed = TRUE;
            continue;
        }

        output = fopen(outputPath, "w");
        if (output == NULL)
        {
            fprintf(stderr, "Cannot open output %s\n", outputPath);
            fclose(input);
            free(inputPath);
            free(expectedPath);
            free(outputPath);
            failed = TRUE;
            continue;
        }

        caseResult = runCase(input, output, name);
        fclose(input);
        fclose(output);

        if (caseResult != 0)
        {
            printf("FAIL: %s (invalid test fixture)\n", name);
            failed = TRUE;
        }
        else if (compareFiles(outputPath, expectedPath))
            printf("PASS: %s\n", name);
        else
        {
            printf("FAIL: %s (output: %s, expected: %s)\n",
                   name, outputPath, expectedPath);
            failed = TRUE;
        }

        free(inputPath);
        free(expectedPath);
        free(outputPath);
    }

    fclose(list);
    fprintf(stderr, "[%s TEST] Processed %d tests\n", suiteLabel, testCount);

    if (failed)
    {
        fprintf(stderr, "[%s TEST] Some tests failed\n", suiteLabel);
        return 2;
    }

    printf("All %s tests passed.\n", suiteLabel);
    return 0;
}
