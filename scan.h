/*
File: scan.h
The scanner interface for the C-minus compiler
*/

#ifndef _SCAN_H_
#define _SCAN_H_

#include "globals.h"
/* MAXTOKENLEN = maximum token length (C99 standard)*/
#define MAXTOKENLEN 32

/* tokenString array stores the lexeme of each token */
extern char tokenString[MAXTOKENLEN + 1];

/* tokenLine: line number associated with the most recently returned token (1-based) */
extern int tokenLine;

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