#ifndef PDB_TCAPPARSER_STOREOPERATION_H
#define PDB_TCAPPARSER_STOREOPERATION_H

#include <memory>
#include <vector>

#include "BuildTcapIrTests.h"
#include "TableAssignment.h"
#include "TcapIdentifier.h"
#include "TcapStatement.h"

using std::shared_ptr;
using std::vector;

namespace pdb_detail {
/**
 * Models a StoreOperation in the TCAP grammar.  For example:
 *
 *    store A[1,2] "(databaseName, outputSetName)"
 *
 * In this example:
 *
 *     outputTable would be A
 *     columnsToStore would be [1,2]
 *     destination would be (databaseName, outputSetName)
 */
class StoreOperation : public TcapStatement {
public:
    /**
     * The table from which columns are to be stored.
     */
    const TcapIdentifier outputTable;

    /**
     * The columns to store.
     */
    const shared_ptr<const vector<TcapIdentifier>> columnsToStore;

    /**
     * The storage location descriptor.
     */
    const string destination;

    // contract from super
    void match(function<void(TableAssignment&)>, function<void(StoreOperation&)> forStore);

private:
    /**
    * Creates a new StoreOperation.
    *
    * If columnsToStore is nullptr, throws invalid_argument exception.
    *
    * @param outputTable The table from which columns are to be stored.
    * @param columnsToStore The columns to store.
    * @param destination The storage location descriptor.
    * @return a new StoreOperation
    */
    // private because throws exception and PDB style guile forbids exceptions from crossing API
    // boundaries.
    StoreOperation(TcapIdentifier outputTable,
                   shared_ptr<vector<TcapIdentifier>> columnsToStore,
                   const string& destination);

    /**
     * Creates a new StoreOperation with attributes
     *
     * If columnsToStore is nullptr, throws invalid_argument exception.
     * If attributes is nullptr, throws invalid_argument exception.
     *
     * @param attributes the attributes of the statement
     * @param outputTable The table from which columns are to be stored.
     * @param columnsToStore The columns to store.
     * @param destination The storage location descriptor.
     * @return a new StoreOperation
     */
    // private because throws exception and PDB style guile forbids exceptions from crossing API
    // boundaries.
    StoreOperation(shared_ptr<vector<TcapAttribute>> attributes,
                   TcapIdentifier outputTable,
                   shared_ptr<vector<TcapIdentifier>> columnsToStore,
                   const string& destination);

    // for constructor
    friend shared_ptr<StoreOperation> makeStoreOperation(
        class TcapTokenStream& tokens, shared_ptr<vector<TcapAttribute>> attributes);

    // for constructor
    friend void pdb_tests::buildTcapIrTest7(class UnitTest& qunit);
};

typedef shared_ptr<StoreOperation> StoreOperationPtr;
}

#endif  // PDB_STOREOPERATION_H
