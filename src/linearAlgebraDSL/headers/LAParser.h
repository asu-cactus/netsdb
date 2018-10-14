#ifndef LA_PARSER_H
#define LA_PARSER_H

// by Binhang, June 2017

#include "LALexer.h"

struct LAStatementsList;

#ifdef __cplusplus
extern "C"
#endif


    int
    LAparse(LAscan_t, struct LAStatementsList**);


#endif