
#ifndef CHAR_PTR
#define CHAR_PTR(c) ((char*)c)
#endif
#ifndef REF_COUNT_PREAMBLE_SIZE
#define REF_COUNT_PREAMBLE_SIZE (sizeof(unsigned))
#endif
#ifndef NUM_COPIES
#define NUM_COPIES (*((unsigned*)this))
#endif
