#include "TcapToken.h"

namespace pdb_detail {
TcapToken::TcapToken(const string& lexeme, const TcapTokenType& tokenType)
    : lexeme(lexeme), tokenType(tokenType) {}
}
