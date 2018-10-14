#include "TranslationUnit.h"

using std::make_shared;
using std::invalid_argument;

namespace pdb_detail {
/**
 * @param statement the statement to wrap in a vector.
 * @return a vector containing only statement
 */
shared_ptr<vector<TcapStatementPtr>> wrapInVector(TcapStatementPtr statement) {
    shared_ptr<vector<shared_ptr<TcapStatement>>> stmts =
        make_shared<vector<shared_ptr<TcapStatement>>>();
    stmts->push_back(statement);
    return stmts;
}

TranslationUnit::TranslationUnit(TcapStatementPtr statement)
    : statements(wrapInVector(statement)) {}

TranslationUnit::TranslationUnit(shared_ptr<const vector<TcapStatementPtr>> statements)
    : statements(statements) {
    if (statements == nullptr)
        throw invalid_argument("statements may not be null");
}
}
