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
                                 std::string createdJobId,
                                 Handle<Computation> dispatchComputation,
                                 Handle<LambdaIdentifier> lambdaIdentifier)
        : databaseName(databaseName), setName(setName), typeName(typeName), pageSize(pageSize), createdJobId(createdJobId) {

       this->dispatchComputation = dispatchComputation;
       this->lambdaIdentifier = lambdaIdentifier;

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

    void setCreatedJobId (std::string createdJobId) {
        this->createdJobId = createdJobId;
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
};
}


#endif  // DISTRIBUTEDSTORAGEADDTEMPSET_H
