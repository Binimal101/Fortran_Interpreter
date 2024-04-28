//
// Created by Matthew Tujague on 4/27/2024.
//

#ifndef PA3_TUJAGUE_M_PA3_H
#define PA3_TUJAGUE_M_PA3_H

#include "lex.h"
#include <set>
#include <stack>

extern LexItem li;
extern LexItem lookahead;
extern int errct;

extern bool isIFRead;
extern bool isSimpleIF;
extern int ifDepth;
extern stack<string> ifDepthMsg;

extern bool isDecl;
extern set<string> defVar;

#endif //PA3_TUJAGUE_M_PA3_H
