#ifndef PDB_TCAPPARSER_TCAPSTATEMENT_H
#define PDB_TCAPPARSER_TCAPSTATEMENT_H

#include <functional>
#include <memory>
#include <vector>

#include "TcapAttribute.h"

using std::function;
using std::shared_ptr;
using std::vector;

namespace pdb_detail {
class TableAssignment;  // forward declaration for match

class StoreOperation;  // forward declaration for match

/**
 * Base class for a TCAP statement, which may be one of:
 *
 *     TableAssignment
 *     StoreOperation
 */
class TcapStatement {
public:
    /**
     * The attributes of the statement.
     */
    const shared_ptr<const vector<TcapAttribute>> attributes;

    /**
     * Creates a TcapStatement with one attribute.
     *
     * @param attribute the attribute
     * @return a new TcapStatement
     */
    TcapStatement(TcapAttribute attribute);

    /**
     * Creates a TcapStatement with zero or  more attributes.
     *
     * If attributes is nullptr, throws invalid_argument exception.
     *
     * @param attributes the attributes
     * @return a new TcapStatement
     */
    TcapStatement(shared_ptr<vector<TcapAttribute>> attributes);

    /**
     * If the TcapStatement is an instance of TableAssignment, call forTableAssignment.
     * If the TcapStatement is an instance of StoreOperation, call forStore.
     *
     * @param forTableAssignment the TableAssignment case
     * @param forStore the StoreOperation case
     */
    virtual void match(function<void(TableAssignment&)> forTableAssignment,
                       function<void(StoreOperation&)> forStore) = 0;
};

typedef shared_ptr<TcapStatement> TcapStatementPtr;
}

#endif  // PDB_TCAPPARSER_IDENTIFIER_H
