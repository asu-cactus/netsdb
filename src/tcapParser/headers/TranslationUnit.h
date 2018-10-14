#ifndef PDB_TCAPPARSER_TRANSLATIONUNIT_H
#define PDB_TCAPPARSER_TRANSLATIONUNIT_H

#include <memory>
#include <vector>

#include "TcapStatement.h"
#include "SafeResult.h"

using std::make_shared;
using std::shared_ptr;
using std::vector;

namespace pdb_detail {
/**
 * A list of TCAP statements.
 */
class TranslationUnit {

public:
    /**
     * The statements of the unit.
     */
    const shared_ptr<const vector<TcapStatementPtr>> statements;

    /**
     * Creates a single statement unit.
     *
     * @param statement the statement
     * @return A new TranslationUnit
     */
    TranslationUnit(TcapStatementPtr statement);

private:
    /**
     * Creates a new TranslationUnit
     *
     * If statements is nullptr, throws invalid_argument exception.
     *
     * @param statements the statements of the unit.
     * @return a new TranslationUnit
     */
    // private because throws exception and PDB style guide forbids exceptions crossing API
    // boundaries.
    TranslationUnit(shared_ptr<const vector<TcapStatementPtr>> statements);

    friend shared_ptr<pdb::SafeResult<TranslationUnit>> parseTcap(
        const string& source);  // for constructor
};
}

#endif  // PDB_TCAPPARSER_TRANSLATIONUNIT_H
