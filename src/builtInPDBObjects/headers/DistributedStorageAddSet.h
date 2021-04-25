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
                             Handle<LambdaIdentifier> lambdaIdentifier = nullptr,
                             size_t desiredSize = 1,
                             bool isMRU = false, bool share=false)
        : dataBase(dataBase), setName(setName), typeName(typeName), pageSize(pageSize), createdJobId(createdJobId) {

        this->dispatchComputation = dispatchComputation;
        this->lambdaIdentifier = lambdaIdentifier;
        this->desiredSize = desiredSize;
        this->isMRU = isMRU;
        this->share = share;
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

    size_t getDesiredSize() {
        return this->desiredSize;
    }

    void setDesiredSize(size_t desiredSize) {
        this->desiredSize = desiredSize;
    }

    bool getMRUorNot() {
        return this->isMRU;
    }

    void setMRUorNot(bool isMRU) {
        this->isMRU = isMRU;
    }

    bool getShare() {
        return this->share;
    }

    void setShare(bool share) {
        this->share = share;
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
    size_t desiredSize;
    bool isMRU;
    bool share;
};
}


#endif  // DISTRIBUTEDSTORAGEADDSET_H
