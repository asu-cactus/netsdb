
#ifndef DELETE_SET_H
#define DELETE_SET_H

#include <vector>
#include <iostream>
#include <memory>
#include "Query.h"

// PRELOAD %DeleteSet%

namespace pdb {

// this corresponds to a database set
class DeleteSet : public Object {

public:
    ENABLE_DEEP_COPY

    DeleteSet() {}
    ~DeleteSet() {}

    DeleteSet(std::string dbNameIn, std::string setNameIn) {
        dbName = dbNameIn;
        setName = setNameIn;
    }

    std::string whichDatabase() {
        return dbName;
    }

    std::string whichSet() {
        return setName;
    }

private:
    String dbName;
    String setName;
};
}

#endif
