#ifndef PDB_TCAPLEXER_LEXEME_H
#define PDB_TCAPLEXER_LEXEME_H

#include <memory>
#include <string>

#include "TcapTokenType.h"

using std::shared_ptr;
using std::string;

namespace pdb_detail {
/**
 * A lexcial unit and its token type value.
 *
 * One of the terminals of the TCAP grammar:
 *
 * https://github.com/riceplinygroup/pdb/wiki/TCAP-Grammar
 *
 * Or, "uknown" which has type TcapTokenType::UKNOWN and lexeme "".
 */
class TcapToken {

public:
    /**
     * The content of the token.
     */
    const string lexeme;

    /**
     * The token's type.
     */
    const TcapTokenType tokenType;

    /**
     * Creates a new TcapToken.
     *
     * @param lexeme the token's content
     * @param tokenType the token's type
     * @return a new TcapToken
     */
    TcapToken(const string& lexeme, const TcapTokenType& tokenType);
};

typedef shared_ptr<TcapToken> TcapTokenPtr;
}

#endif  // PDB_TCAPLEXER_LEXEME_H
