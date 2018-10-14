//
// Created by Joseph Hwang on 9/25/16.
//

#ifndef OBJECTQUERYMODEL_DISPATCHERREGISTERPARTITIONPOLICY_H
#define OBJECTQUERYMODEL_DISPATCHERREGISTERPARTITIONPOLICY_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

#include "PartitionPolicy.h"

// PRELOAD %DispatcherRegisterPartitionPolicy%

namespace pdb {

class DispatcherRegisterPartitionPolicy : public Object {

public:
    DispatcherRegisterPartitionPolicy() {}
    ~DispatcherRegisterPartitionPolicy() {}

    DispatcherRegisterPartitionPolicy(std::string setNameIn,
                                      std::string databaseNameIn,
                                      PartitionPolicy::Policy policyIn)
        : setName(setNameIn), databaseName(databaseNameIn), policy(policyIn) {}

    String getSetName() {
        return this->setName;
    }

    String getDatabaseName() {
        return this->databaseName;
    }

    PartitionPolicy::Policy getPolicy() {
        return this->policy;
    }

    ENABLE_DEEP_COPY

private:
    String setName;
    String databaseName;
    PartitionPolicy::Policy policy;
};
}


#endif  // OBJECTQUERYMODEL_DISPATCHERREGISTERPARTITIONPOLICY_H
