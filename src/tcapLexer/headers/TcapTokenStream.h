
#ifndef PDB_TCAPLEXER_TOKENSTREAM_H
#define PDB_TCAPLEXER_TOKENSTREAM_H

#include <memory>
#include <vector>

#include "TcapToken.h"

using std::shared_ptr;
using std::vector;

using pdb_detail::TcapToken;

namespace pdb_detail {
/**
 * A sequence of tokens.
 */
class TcapTokenStream {
public:
    /**
     * @return true if any tokens remain, else false.
     */
    bool hasNext();

    /**
     * Returns the current token and advances the stream to the next token.
     *
     * @return the next token in the stream or an empty string token with type UNKNOWN if all tokens
     * have
     * been consumed.
     */
    TcapToken advance();

    /**
     * Returns the current token of the stream without advancing the stream.
     *
     * @return the next token in the stream or an empty string token with type UNKNOWN if all tokens
     * have
     * been consumed.
     */
    TcapToken peek();

private:
    /**
     * The index of the current token of the stream in _tokens.
     */
    vector<TcapToken>::size_type _readIndex = 0;

    /**
     * The tokens of the stream.
     */
    shared_ptr<const vector<TcapToken>> _tokens;

    /**
     * Constructs a stream from the given tokens.
     *
     * Throws invalid_argument exception if tokens is null.
     *
     * @param tokens the tokens of the stream. may not be null.
     * @return a new token stream
     */
    // private because throws exception and PDB style guide says no excepsions across API boundaries
    TcapTokenStream(shared_ptr<const vector<TcapToken>> tokens);

    friend TcapTokenStream lexTcap(const string& source);  // for access to constructor
};
}

#endif  // PDB_TCAPLEXER_TOKENSTREAM_H
