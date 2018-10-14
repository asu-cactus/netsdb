#ifndef PDB_TCAPPARSER_LOADOPERATION_H
#define PDB_TCAPPARSER_LOADOPERATION_H

#include <functional>
#include <memory>
#include <string>

#include "TableExpression.h"

using std::shared_ptr;
using std::string;

namespace pdb_detail {
/**
 * Models a LoadOperation in the TCAP grammar.  For example:
 *
 *    load "(databaseName, inputSetName)"
 *
 * In this example:
 *
 *     source would be (databaseName, inputSetName)
 */
class LoadOperation : public TableExpression {
public:
    /**
     * The source of the load.
     */
    const string source;

    /**
     * Creates a new LoadOperation.
     * @param source The source of the load.
     * @return the new LoadOperation
     */
    LoadOperation(const string& source);

    // contract from super
    void match(function<void(LoadOperation&)> forLoad,
               function<void(ApplyOperation&)>,
               function<void(FilterOperation&)>,
               function<void(HoistOperation&)>,
               function<void(BinaryOperation&)>) override;
};

typedef shared_ptr<LoadOperation> LoadOperationPtr;
}

#endif  // PDB_TCAPPARSER_LOADOPERATION_H
