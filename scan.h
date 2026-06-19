/*
File: scan.h
The scanner interface for the C-minus compiler
*/

#ifndef _SCAN_H_
#define _SCAN_H_

#include "globals.h"
/* MAXTOKENLEN = maximum token length (C99 standard)*/
#define MAXTOKENLEN 32

typedef enum
{
    SCAN_ERROR_NONE,
    SCAN_ERROR_INVALID_SYMBOL,
    SCAN_ERROR_MALFORMED_ID,
    SCAN_ERROR_MALFORMED_NUM,
    SCAN_ERROR_UNMATCHED_COMMENT_END,
    SCAN_ERROR_UNTERMINATED_COMMENT
} ScanErrorKind;

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];

/* tokenLine: line number associated with the most recently returned token (1-based) */
extern int tokenLine;

/* scanErrorKind describes the most recently returned ERROR token. */
extern ScanErrorKind scanErrorKind;

/* Prints a diagnostic for the most recently returned ERROR token. */
void reportScanError(FILE *out);

/* getToken function
Args: none
Returns: the next token in source file

 */
TokenType getToken(void);

/* resetScanner function
 * Clears the scanner's internal buffered state before starting a new source file.
 */
void resetScanner(void);
#endif
