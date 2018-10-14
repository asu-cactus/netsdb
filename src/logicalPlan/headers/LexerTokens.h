
#ifndef LEX_TOKENS_H
#define LEX_TOKENS_H

#ifdef FILTER
#undef FILTER
#undef APPLY
#undef SCAN
#undef AGG
#undef JOIN
#undef OUTPUT
#undef GETS
#undef IDENTIFIER
#undef STRING
#undef HASHLEFT
#undef HASHRIGHT
#undef HASHONE
#undef FLATTEN
#endif

#define FILTER 258
#define APPLY 259
#define SCAN 260
#define AGG 261
#define JOIN 262
#define OUTPUT 263
#define GETS 264
#define HASHLEFT 265
#define HASHRIGHT 266
#define HASHONE 267
#define FLATTEN 268
#define IDENTIFIER 269
#define STRING 270

#endif
