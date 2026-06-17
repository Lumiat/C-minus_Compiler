/*
File: util.h
Unility functions for the C-minus compiler
*/

#ifndef _UTIL_H_
#define _UTIL_H_
#include "globals.h"

/* Procedure printToken prints a token
 * and its lexeme to the listing file
 */
void printToken(TokenType, const char *);

/* Procedure printSourceLine prints a numbered source line to the output file */
void printSourceLine(FILE *out, int lineNumber, const char *lineText);

/* AST node constructors */
TreeNode *newDeclNode(DeclKind);
TreeNode *newStmtNode(StmtKind);
TreeNode *newExpNode(ExpKind);

/* Utility helpers */
char *copyString(const char *s);
void printTree(TreeNode *tree);

#endif