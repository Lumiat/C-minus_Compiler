#include "globals.h"

FILE *source = NULL;
FILE *listing = NULL;
FILE *code = NULL;

int lineno = 0;

int EchoSource = FALSE;
int TraceScan = FALSE;
int TraceParse = FALSE;
int TraceAnalyze = FALSE;
int TraceCode = FALSE;
int Error = FALSE;
