#ifndef DISTRIBUTEDSTORAGEADDSETWITHPARTITION_H
#define DISTRIBUTEDSTORAGEADDSETWITHPARTITION_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "Computation.h"
#include "LambdaIdentifier.h"


// PRELOAD %DistributedStorageAddSetWithPartition%

namespace pdb {

// encapsulates a request to add a set in storage
class DistributedStorageAddSetWithPartition : public Object {

public:
    DistributedStorageAddSetWithPartition() {}
    ~DistributedStorageAddSetWithPartition() {}


    DistributedStorageAddSetWithPartition(std::string dataBase,
                             std::string setName,
                             std::string typeName,
                             size_t pageSize,
                             std::string createdJobId,
                             Handle<Vector<Handle<Computation>>> dispatchComputations,
                             std::string jobName,
                             size_t desiredSize = 1,
                             bool isMRU = false)
        : dataBase(dataBase), setName(setName), typeName(typeName), pageSize(pageSize), createdJobId(createdJobId) {

        this->dispatchComputations = dispatchComputations;
        this->jobName = jobName;
        this->desiredSize = desiredSize;
        this->isMRU = isMRU;
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

    Handle<Vector<Handle<Computation>>>  getDispatchComputations() {
        return this->dispatchComputations;
    }

    void setDispatchComputation(Handle<Vector<Handle<Computation>>>  dispatchComputations) {
        this->dispatchComputations = dispatchComputations;
    }

    std::string  getJobName() {
        return this->jobName;
    }

    void setJobName(std::string jobName) {
        this->jobName = jobName;
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

    ENABLE_DEEP_COPY

private:
    String dataBase;
    String setName;
    String typeName;
    String createdJobId;
    size_t pageSize;
    Handle<Vector<Handle<Computation>>> dispatchComputations;
    String jobName;
    size_t desiredSize;
    bool isMRU;
};
}


#endif  // DISTRIBUTEDSTORAGEADDSETWITHPARTITION_H
