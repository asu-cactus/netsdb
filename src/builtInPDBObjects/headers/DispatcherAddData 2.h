//
// Created by Joseph Hwang on 9/12/16.
//

#ifndef OBJECTQUERYMODEL_DISPATCHERADDDATA_H
#define OBJECTQUERYMODEL_DISPATCHERADDDATA_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %DispatcherAddData%

namespace pdb {

// encapsulates a request to add data to a set in storage
class DispatcherAddData : public Object {

public:
    DispatcherAddData() {}
    ~DispatcherAddData() {}

    DispatcherAddData(std::string databaseName,
                      std::string setName,
                      std::string typeName,
                      bool shallowCopyOrNot = false)
        : databaseName(databaseName), setName(setName), typeName(typeName) {
        this->shallowCopyOrNot = shallowCopyOrNot;
    }

    std::string getDatabaseName() {
        return databaseName;
    }

    std::string getSetName() {
        return setName;
    }

    std::string getTypeName() {
        return typeName;
    }

    bool isShallowCopy() {
        return shallowCopyOrNot;
    }

    ENABLE_DEEP_COPY

private:
    String databaseName;
    String setName;
    String typeName;
    bool shallowCopyOrNot;
};
}

#endif  // OBJECTQUERYMODEL_DISPATCHERADDDATA_H
