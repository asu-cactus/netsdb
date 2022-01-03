#ifndef DISPATCHERGETSETRESULT_H
#define DISPATCHERGETSETRESULT_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

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

        explicit DispatcherGetSetResult(const std::string &database, const std::string &set, const std::string &internalType, const std::string &type) : databaseName(database),
                                                      setName(set), internalType(internalType),type(type) {}

        ENABLE_DEEP_COPY

        /**
         * The name of the database
         */

        String databaseName;

        /**
         * The name of the set
         */

        String setName;

        /**
         * The the name of the internal type that is going to be handling the types.
         * For example a pdb::Vector<StringIntPair> is going to be handled by pdb::Vector<pdb::Nothing>
         */

        String internalType;

        /**
         * The real name of the type see above
         */

        String type;
    };
}

#endif