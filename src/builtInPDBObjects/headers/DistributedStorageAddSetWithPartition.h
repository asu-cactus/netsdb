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
                             std::string jobName1,
                             std::string jobName2,
                             std::string computationName1,
                             std::string computationName2,
                             std::string lambdaName1,
                             std::string lambdaName2,
                             size_t desiredSize = 1,
                             bool isMRU = false,
			     bool isSharedTensorBlockSet = false,
			     bool isModelSet = false)
        : dataBase(dataBase), setName(setName), typeName(typeName), pageSize(pageSize), createdJobId(createdJobId) {

        this->dispatchComputations = dispatchComputations;
        this->jobName = jobName;
        this->jobName1 = jobName1;
        this->jobName2 = jobName2;
        this->computationName1 = computationName1;
        this->computationName2 = computationName2;
        this->lambdaName1 = lambdaName1;
        this->lambdaName2 = lambdaName2;
        this->desiredSize = desiredSize;
        this->isMRU = isMRU;
	this->isSharedTensorBlockSet = isSharedTensorBlockSet;
	this->isModelSet = isModelSet;
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

    std::string  getJobName1() {
        return this->jobName1;
    }

    void setJobName1(std::string jobName1) {
        this->jobName1 = jobName1;
    }

    std::string  getJobName2() {
        return this->jobName2;
    }

    void setJobName2(std::string jobName2) {
        this->jobName2 = jobName2;
    }

    std::string  getLambdaName1() {
        return this->lambdaName1;
    }

    void setLambdaName1(std::string lambdaName1) {
        this->lambdaName1 = lambdaName1;
    }

    std::string  getLambdaName2() {
        return this->lambdaName2;
    }

    void setLambdaName2(std::string lambdaName2) {
        this->lambdaName2 = lambdaName2;
    }

    std::string  getComputationName1() {
        return this->computationName1;
    }

    void setComputationName1(std::string computationName1) {
        this->computationName1 = computationName1;
    }

    std::string  getComputationName2() {
        return this->computationName2;
    }
    
    void setComputationName2(std::string computationName2) {
        this->computationName2 = computationName2;
    }

    bool getSharedTensorBlockSet() {
        return this->isSharedTensorBlockSet;
    }

    void setSharedTensorBlockSet(bool isSharedTensorBlockSet) {
        this->isSharedTensorBlockSet = isSharedTensorBlockSet;
    }

    bool getModelSet() {
        return isModelSet;
    }

    void setModelSet(bool isThisModelSet) {
        this->isModelSet = isThisModelSet;
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
    String jobName1;
    String jobName2;
    String computationName1;
    String computationName2;
    String lambdaName1;
    String lambdaName2;
    bool isSharedTensorBlockSet;
    bool isModelSet;
};
}


#endif  // DISTRIBUTEDSTORAGEADDSETWITHPARTITION_H
