#ifndef _TEST_RUNNER_H_
#define _TEST_RUNNER_H_

#include <stdio.h>

typedef int (*TestCaseFunction)(FILE *input, FILE *output, const char *name);

int runTestSuite(const char *suiteLabel, const char *baseDirectory,
                 TestCaseFunction runCase);

#endif
