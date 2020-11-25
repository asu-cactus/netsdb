
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
#undef PARTITION
#endif

#define FILTER 258
#define APPLY 259
#define SCAN 260
#define AGG 261
#define JOIN 262
#define OUTPUT 263
#define GETS 264
#define PARTITION 265
#define HASHLEFT 266
#define HASHRIGHT 267
#define HASHONE 268
#define FLATTEN 269
#define IDENTIFIER 270
#define STRING 271

#endif
