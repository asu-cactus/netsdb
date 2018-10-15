#ifndef DISTRIBUTEDSTORAGEADDTEMPSET_H
#define DISTRIBUTEDSTORAGEADDTEMPSET_H

// by Jia, Mar 2017

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "Computation.h"
#include "LambdaIdentifier.h"

// PRELOAD %DistributedStorageAddTempSet%

namespace pdb {

// encapsulates a request to add a set in storage
class DistributedStorageAddTempSet : public Object {

public:
    DistributedStorageAddTempSet() {}
    ~DistributedStorageAddTempSet() {}


    DistributedStorageAddTempSet(std::string databaseName,
                                 std::string setName,
                                 std::string typeName,
                                 size_t pageSize,
                                 std::string createdJobId = "",
                                 Handle<Computation> dispatchComputation = nullptr,
                                 Handle<LambdaIdentifier> lambdaIdentifier = nullptr,
                                 size_t desiredSize = 1)
        : databaseName(databaseName), setName(setName), typeName(typeName), pageSize(pageSize), createdJobId(createdJobId) {

       this->dispatchComputation = dispatchComputation;
       this->lambdaIdentifier = lambdaIdentifier;
       this->desiredSize = desiredSize;

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

    size_t getPageSize() {
        return pageSize;
    }

    std::string getCreatedJobId() {
        return this->createdJobId;
    }

    void setCreatedJobId(std::string createdJobId) {
        this->createdJobId = createdJobId;
    }

    size_t getDesiredSize() {
        return this->desiredSize;
    }

    void setDesiredSize(size_t desiredSize) {
        this->desiredSize = desiredSize;
    }



    ENABLE_DEEP_COPY

private:
    String databaseName;
    String setName;
    String typeName;
    size_t pageSize;
    String createdJobId;
    Handle<Computation> dispatchComputation;
    Handle<LambdaIdentifier> lambdaIdentifier;
    size_t desiredSize;
};
}


#endif  // DISTRIBUTEDSTORAGEADDTEMPSET_H
