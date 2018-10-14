#ifndef PDB_TCAPLEXER_TOKENTYPE_H
#define PDB_TCAPLEXER_TOKENTYPE_H

/**
 * The types of TCAP tokens.
 */
enum TcapTokenType {
    UNKNOWN_TYPE,  // an unexpected lexeme in the string

    STRING_LITERAL_TYPE,  // "\"*"

    IDENTIFIER_TYPE,  // (letter | digit)+   except a keyword

    LPAREN_TYPE,  // (

    RPAREN_TYPE,  // )

    EQ_TYPE,  // =

    LOAD_TYPE,  // load

    APPLY_TYPE,  // apply

    TO_TYPE,  // to

    LBRACKET_TYPE,  // [

    RBRACKET_TYPE,  // ]

    RETAIN_TYPE,  // retain

    ALL_TYPE,  // all

    COMMA_TYPE,  // ,

    BY_TYPE,  // by

    STORE_TYPE,  // store

    FILTER_TYPE,  // filter

    NONE_TYPE,  // none

    AT_SIGN_TYPE,  // @

    FUNC_TYPE,  // func

    METHOD_TYPE,  // method

    HOIST_TYPE,  // hoist

    FROM_TYPE,  // from

    GREATER_THAN_TYPE  // >
};

#endif  // PDB_TCAPLEXER_TOKENTYPE_H
