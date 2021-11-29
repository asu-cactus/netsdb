#ifndef _PARSER_H_
#define _PARSER_H_

#include "Lexer.h"

struct AtomicComputationList;

#ifdef __cplusplus
extern "C"
#endif
    int
    yyparse(yyscan_t, struct AtomicComputationList**);

#endif
