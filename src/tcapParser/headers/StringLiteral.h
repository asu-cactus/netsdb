#ifndef PDB_TCAPPARSER_STRINGLITERAL_H
#define PDB_TCAPPARSER_STRINGLITERAL_H

#include <memory>
#include <string>

using std::shared_ptr;
using std::string;

namespace pdb_detail {
/**
 * A TCAP string literal.
 */
class StringLiteral {

public:
    /**
     * The contents of the literal, without the enclosing quotes.
     */
    const string contents;

    /**
     * Creates a new string literal.
     *
     * @param contents The unquoated contests of the literal.
     * @return A new String literal.
     */
    StringLiteral(const string& contents);
};
}

#endif  // PDB_TCAPPARSER_STRINGLITERAL_H
