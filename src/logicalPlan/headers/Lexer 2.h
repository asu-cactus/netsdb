#ifndef _LABCEXER_H_
#define _LABCEXER_H_

#include "LexerTokens.h"

#ifdef __cplusplus
extern "C" {
#endif

struct LexerExtra {
    char errorMessage[500];
};

typedef void* yyscan_t;
int yylex_init_extra(struct LexerExtra*, yyscan_t*);
int yylex_destroy(yyscan_t);

typedef struct yy_buffer_state* YY_BUFFER_STATE;
YY_BUFFER_STATE yy_scan_string(const char*, yyscan_t);
void yy_delete_buffer(YY_BUFFER_STATE, yyscan_t);

struct AtomicComputationList;
void yyerror(yyscan_t, struct AtomicComputationList** myStatement, const char*);

// union YYSTYPE;
typedef union
#ifdef __cplusplus
    YYSTYPE
#endif
{
    char* myChar;
    struct AtomicComputationList* myAtomicComputationList;
    struct AtomicComputation* myAtomicComputation;
    struct TupleSpec* myTupleSpec;
    struct AttList* myAttList;
} YYSTYPE;

extern YYSTYPE yylval;
int yylex(YYSTYPE*, yyscan_t);


#ifdef __cplusplus
}
#endif

#endif
