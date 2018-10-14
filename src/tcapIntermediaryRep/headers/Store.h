#ifndef PDB_TCAPINTERMEDIARYREP_STORE_H
#define PDB_TCAPINTERMEDIARYREP_STORE_H

#include "ApplyFunction.h"
#include "Instruction.h"

namespace pdb_detail {
/**
 * An instruction to copy table columns to an implimentation specific destination.
 */
class Store : public Instruction {
public:
    /**
     * The table/columns to store.
     */
    const TableColumns columnsToStore;

    /**
     * An implimentation specific destinaton descriptor.
     */
    const string destination;

    /**
     * Creates a new Store instruction.
     *
     * @param columnsToStore  The table/columns to store.
     * @param destination An implimentation specific destinaton descriptor.
     * @return the new Store instruction
     */
    Store(TableColumns columnsToStore, const string& destination);

    // contract from super
    void match(function<void(Load&)>,
               function<void(ApplyFunction&)>,
               function<void(ApplyMethod&)>,
               function<void(Filter&)>,
               function<void(Hoist&)>,
               function<void(GreaterThan&)>,
               function<void(Store&)> forStore);
};

typedef shared_ptr<Store> StorePtr;

/**
 * Creates a new Store instruction.
 *
 * @param columnsToStore  The table/columns to store.
 * @param destination An implimentation specific destinaton descriptor.
 * @return a shared pointer to the new Store instruction
 */
StorePtr makeStore(TableColumns columnsToStore, const string& destination);
}

#endif  // PDB_TCAPINTERMEDIARYREP_STORE_H
