#ifndef LA_LEX_H
#define LA_LEX_H


#include "LALexerTokens.h"

// by Binhang, June 2017

#ifdef __cplusplus
extern "C" {
#endif

typedef void* LAscan_t;

void LAerror(LAscan_t, const char*);
int LAlex_destroy(LAscan_t);
int LAlex_init(LAscan_t*);
void LAset_in(FILE*, LAscan_t);

// union YYSTYPE;
typedef union
#ifdef __cplusplus
    YYSTYPE
#endif
{
    char* stringVal;
    double doubleVal;
    int intVal;
} YYSTYPE;

extern YYSTYPE yylval;

int LAlex(YYSTYPE*, LAscan_t);


#ifdef __cplusplus
}
#endif

#endif