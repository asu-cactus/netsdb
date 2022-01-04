#ifndef DISPATCHERGETSETRESULT_H
#define DISPATCHERGETSETRESULT_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %DispatcherGetSetResult%

namespace pdb {

    /**
     * Encapsulates a request to search for a set in the database
     */

    class DispatcherGetSetResult : public Object {

    public:

        DispatcherGetSetResult() = default;
        ~DispatcherGetSetResult() = default;

        /**
         * Creates a request to get the database
         * @param database - the name of database
         */

        explicit DispatcherGetSetResult(const std::string &database, const std::string &set) : databaseName(database),
                                                      setName(set) {}

        ENABLE_DEEP_COPY

        /**
         * The name of the database
         */

        String databaseName;

        /**
         * The name of the set
         */

        String setName;

        std::string getDatabaseName() {
            return databaseName;
        }

        std::string getSetName() {
            return setName;
        }
    };
}

#endif