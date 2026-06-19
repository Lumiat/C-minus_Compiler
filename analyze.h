/*
File: analyze.h
The semantic analyzer interface for the C-minus compiler
*/

#ifndef _ANALYZE_H_
#define _ANALYZE_H_

#include "globals.h"

void analyze(TreeNode *syntaxTree);
void buildSymtab(TreeNode *syntaxTree);
void typeCheck(TreeNode *syntaxTree);

#endif /* _ANALYZE_H_ */
