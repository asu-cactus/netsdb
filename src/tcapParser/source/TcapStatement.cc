#include "TcapStatement.h"

using std::make_shared;
using std::invalid_argument;

namespace pdb_detail {
/**
 * @param attribute the attribute to wrap in a vector
 * @return a vector of only attribute
 */
shared_ptr<vector<TcapAttribute>> wrapInVector(TcapAttribute attribute) {
    shared_ptr<vector<TcapAttribute>> attributes = make_shared<vector<TcapAttribute>>();
    attributes->push_back(attribute);
    return attributes;
}

TcapStatement::TcapStatement(shared_ptr<vector<TcapAttribute>> attributes)
    : attributes(attributes) {
    if (attributes == nullptr)
        throw invalid_argument("attributes may not be null");
}

TcapStatement::TcapStatement(TcapAttribute attribute) : attributes(wrapInVector(attribute)) {}
}
