#include <iostream>
#include "TcapTokenStream.h"

using std::invalid_argument;

namespace pdb_detail {

TcapTokenStream::TcapTokenStream(shared_ptr<const vector<TcapToken>> tokens) : _tokens(tokens) {
    if (tokens == nullptr)
        throw invalid_argument("tokens may not be null");
}

bool TcapTokenStream::hasNext() {
    return _readIndex < _tokens->size();
}

TcapToken TcapTokenStream::advance() {
    if (_readIndex >= _tokens->size())
        return TcapToken("", TcapTokenType::UNKNOWN_TYPE);

    return _tokens->operator[](_readIndex++);
}

TcapToken TcapTokenStream::peek() {
    if (_readIndex >= _tokens->size())
        return TcapToken("", TcapTokenType::UNKNOWN_TYPE);

    return _tokens->operator[](_readIndex);
}
}