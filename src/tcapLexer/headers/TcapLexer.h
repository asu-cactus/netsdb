#ifndef PDB_TCAPLEXER_TCAPLEXER_H
#define PDB_TCAPLEXER_TCAPLEXER_H

#include "TcapTokenStream.h"

namespace pdb_detail {
/**
 * Tokenize a string into a token stream.
 *
 * Unrecognized tokens will be reported as TokenType::UNKNOWN_TYPE
 *
 * @param source the input string
 * @return the tokenization of source
 */
TcapTokenStream lexTcap(const string& source);
}

#endif  // PDB_TCAPLEXER_TCAPLEXER_H
