#ifndef DISTRIBUTEDSTORAGEADDSET_H
#define DISTRIBUTEDSTORAGEADDSET_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "Computation.h"
#include "LambdaIdentifier.h"


// PRELOAD %DistributedStorageAddSet%

namespace pdb {

// encapsulates a request to add a set in storage
class DistributedStorageAddSet : public Object {

public:
    DistributedStorageAddSet() {}
    ~DistributedStorageAddSet() {}


    DistributedStorageAddSet(std::string dataBase,
                             std::string setName,
                             std::string typeName,
                             size_t pageSize,
                             std::string createdJobId = "",
                             Handle<Computation> dispatchComputation = nullptr,
                             Handle<LambdaIdentifier> lambdaIdentifier = nullptr)
        : dataBase(dataBase), setName(setName), typeName(typeName), pageSize(pageSize), createdJobId(createdJobId) {

        this->dispatchComputation = dispatchComputation;
        this->lambdaIdentifier = lambdaIdentifier;

    }


    std::string getDatabase() {
        return dataBase;
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

    Handle<Computation>  getDispatchComputation() {
        return this->dispatchComputation;
    }

    void setDispatchComputation(Handle<Computation>  dispatchComputation) {
        this->dispatchComputation = dispatchComputation;
    }

    Handle<LambdaIdentifier>  getLambdaIdentifier() {
        return this->lambdaIdentifier;
    }

    void setLambdaIdentifier(Handle<LambdaIdentifier>  lambdaIdentifier) {
        this->lambdaIdentifier = lambdaIdentifier;
    }

    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    String typeName;
    String createdJobId;
    size_t pageSize;
    Handle<Computation> dispatchComputation;
    Handle<LambdaIdentifier> lambdaIdentifier;
};
}


#endif  // DISTRIBUTEDSTORAGEADDSET_H
