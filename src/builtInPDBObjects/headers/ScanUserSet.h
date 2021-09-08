
#ifndef SCAN_USER_SET_H
#define SCAN_USER_SET_H

// PRELOAD %ScanUserSet <Nothing>%

#include "TypeName.h"
#include "Computation.h"
#include "PageCircularBufferIterator.h"
#include "VectorTupleSetIterator.h"
#include "PartitionedVectorTupleSetIterator.h"
#include "PDBString.h"
#include "DataTypes.h"
#include "DataProxy.h"
#include "Configuration.h"
//#include "LambdaHelperClasses.h"

namespace pdb {


//to scan a user set
template <class OutputClass>
class ScanUserSet : public Computation {

public:

    // normally these would be defined by the ENABLE_DEEP_COPY macro, but because
    // Array is the one variable-sized type that we allow, we need to manually override
    // these methods
    void setUpAndCopyFrom(void* target, void* source) const override {
        new (target) ScanUserSet<OutputClass> ();
        ScanUserSet<OutputClass>& fromMe = *((ScanUserSet<OutputClass> *) source);
        ScanUserSet<OutputClass>& toMe = *((ScanUserSet<OutputClass> *) target);
        toMe.iterator = fromMe.iterator;
        toMe.proxy = fromMe.proxy;
        toMe.batchSize = fromMe.batchSize;
        toMe.dbName = fromMe.dbName;
        toMe.setName = fromMe.setName;
        toMe.outputType = fromMe.outputType;
        toMe.computationName = fromMe.computationName;
    }

    void deleteObject(void* deleteMe) override {
        deleter(deleteMe, this);
    }

    size_t getSize(void* forMe) override {
        return sizeof(ScanUserSet<OutputClass>);
    }


    //this constructor is for constructing builtin object
    ScanUserSet () {}



    //user should only use following constructor
    ScanUserSet (std :: string dbName, std :: string setName) {
        this->dbName = dbName;
        this->setName = setName;
        this->outputType = getTypeName<OutputClass>();
    }

    ~ScanUserSet() {
        this->iterator = nullptr;
        this->proxy = nullptr;
    }

    virtual ComputeSourcePtr getComputeSource(TupleSpec& schema, ComputePlan& plan) override {
         if (!isFollowedByLocalJoin()) {
            std::cout << computationName << ": ****Return VectorTupleSetIterator****" << std::endl;
            return std::make_shared<VectorTupleSetIterator>(

            [&]() -> void* {
                if (this->iterator == nullptr) {
                    return nullptr;
                }
                while (this->iterator->hasNext() == true) {

                    PDBPagePtr page = this->iterator->next();
                    if (page != nullptr) {
                        return page->getBytes();
                    } else {
                        sched_yield();
                    }
                }

                return nullptr;

            },

            [&](void* freeMe) -> void {
                
                if (this->proxy != nullptr) {
                    char* pageRawBytes = (char*)freeMe - DEFAULT_PAGE_HEADER_SIZE;

                    PDBPagePtr page = make_shared<PDBPage>(pageRawBytes, 0, 0);
                    NodeID nodeId = page->getNodeID();
                    DatabaseID dbId = page->getDbID();
                    UserTypeID typeId = page->getTypeID();
                    SetID setId = page->getSetID();
                    try {
                        this->proxy->unpinUserPage(nodeId, dbId, typeId, setId, page, false);
                    } catch (NotEnoughSpace& n) {
                        makeObjectAllocatorBlock(4096, true);
                        this->proxy->unpinUserPage(nodeId, dbId, typeId, setId, page, false);
                        throw n;
                    }
                }
            },

            this->batchSize

            );
        } else {
            std::cout << computationName << ": ****Return PartitionedVectorTupleSetIterator****" << std::endl;
            return std::make_shared<PartitionedVectorTupleSetIterator<OutputClass>>(

            myPartitionId,

            numPartitions/numNodes,

            numNodes,

            partitionLambda,


            [&]() -> PDBPagePtr {
                if (this->iterator == nullptr) {
                    return nullptr;
                }
                while (this->iterator->hasNext() == true) {
                    std::cout << myPartitionId << ": to get a page" << std::endl;
                    PDBPagePtr page = this->iterator->next();
                    if (page != nullptr) {
                        std::cout << myPartitionId << ": PartitionedVectorTupleSetIterator got a page with ID=" << page->getPageID() << std::endl;
                        return page;
                    } else {
                        std::cout << myPartitionId << ": PartitionedVectorTupleSetIterator got a null page" << std::endl;
                        sched_yield();
                    }
                }

                return nullptr;

            },

            [&](PDBPagePtr freeMe) -> void {
                if (this->proxy != nullptr) {
                    char* curBytes = freeMe->getRawBytes();
                    NodeID nodeId = (NodeID)(*((NodeID*)(curBytes)));
                    curBytes = curBytes + sizeof(NodeID);
                    DatabaseID dbId = (DatabaseID)(*((DatabaseID*)(curBytes)));
                    curBytes = curBytes + sizeof(DatabaseID);
                    UserTypeID typeId = (UserTypeID)(*((UserTypeID*)(curBytes)));
                    curBytes = curBytes + sizeof(UserTypeID);
                    SetID setId = (SetID)(*((SetID*)(curBytes)));
                    freeMe->decRefCount();
                    if (freeMe->getRefCount() == 0) {
#ifdef PROFILING_CACHE
                        std::cout << myPartitionId << ": To unpin Join source page with DatabaseID=" << dbId
                                  << ", UserTypeID=" << typeId << ", SetID=" << setId
                                  << ", PageID=" << freeMe->getPageID() << std::endl;
#endif
                       
                        try {
                            this->proxy->unpinUserPage(nodeId, dbId, typeId, setId, freeMe, false);
                            std::cout << myPartitionId << ": Unpinned page: "<< freeMe->getPageID() << std::endl;
                        } catch (NotEnoughSpace& n) {
                            makeObjectAllocatorBlock(4096, true);
                            this->proxy->unpinUserPage(nodeId, dbId, typeId, setId, freeMe, false);
                            std::cout << "Unpinned page: "<< freeMe->getPageID() << std::endl;
                        }
                    }
#ifdef PROFILING_CACHE
                    else {
                        std::cout << myPartitionId << ": Can't unpin Join source page with DatabaseID=" << dbId
                                  << ", UserTypeID=" << typeId << ", SetID=" << setId
                                  << ", PageID=" << freeMe->getPageID()
                                  << ", reference count=" << freeMe->getRefCount() << std::endl;
                    }
#endif
                }

            },

            this->batchSize

            );

        }
    }

    // Be careful here that we put PageCircularBufferIteratorPtr and DataProxyPtr in a pdb object
    void setIterator(PageCircularBufferIteratorPtr iterator) {
        this->iterator = iterator;
    }

    void setProxy(DataProxyPtr proxy) {
        this->proxy = proxy;
    }


    void setBatchSize(int batchSize) override {
        this->batchSize = batchSize;
    }

    int getBatchSize() {
        return this->batchSize;
    }

    // set number of partitions  (used in local join)
    void setNumPartitions(int numPartitions) {
        this->numPartitions = numPartitions;
    }

    // return my number of partitions  (used in local join)
    int getNumPartitions() {
        return numPartitions;
    }

    // set number of nodes  (used in local join)
    void setNumNodes(int numNodes) {
        this->numNodes = numNodes;
    }


    // return my number of nodes  (used in local join)
    int getNumNodes() {
        return numNodes;
    }

    // set my partition id for obtaining JoinSource for one partition  (used in local join)
    void setPartitionId(size_t myPartitionId) {
        this->myPartitionId = myPartitionId;
    }

    // return my partition id for obtaining JoinSource for one partition  (used in local
    // join)
    int getPartitionId() {
        return this->myPartitionId;
    }



    void setOutput(std::string dbName, std::string setName) override {
        this->dbName = dbName;
        this->setName = setName;
    }

    void setDatabaseName(std::string dbName) {
        this->dbName = dbName;
    }

    void setSetName(std::string setName) {
        this->setName = setName;
    }

    std::string getDatabaseName() override {
        return dbName;
    }

    std::string getSetName() override {
        return setName;
    }

    


    std::string getComputationType() override {
        std::cout << "***" << this->getComputationName() << std::endl;
        return std::string("ScanUserSet");
    }


    // below function implements the interface for parsing computation into a TCAP string
    std::string toTCAPString(std::vector<InputTupleSetSpecifier>& inputTupleSets,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumnNames,
                             std::string& addedOutputColumnName) override {

        this->setComputationName(getComputationType()+"_"+std::to_string(computationLabel));
        InputTupleSetSpecifier inputTupleSet;
        if (inputTupleSets.size() > 0) {
            inputTupleSet = inputTupleSets[0];
        }
        return toTCAPString(inputTupleSet.getTupleSetName(),
                            inputTupleSet.getColumnNamesToKeep(),
                            inputTupleSet.getColumnNamesToApply(),
                            computationLabel,
                            outputTupleSetName,
                            outputColumnNames,
                            addedOutputColumnName);
    }


    // below function returns a TCAP string for this Computation
    std::string toTCAPString(std::string inputTupleSetName,
                             std::vector<std::string>& inputColumnNames,
                             std::vector<std::string>& inputColumnsToApply,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumnNames,
                             std::string& addedOutputColumnName) {
        outputTupleSetName = "inputDataFor" + getComputationType() + std::string("_") +
            std::to_string(computationLabel);
        addedOutputColumnName = "in" + std::to_string(computationLabel);
        outputColumnNames.push_back(addedOutputColumnName);
        std::string ret = outputTupleSetName +
            std::string("(" + addedOutputColumnName + ") <= SCAN ('") + std::string(dbName) +
            "', '" + std::string(setName) + std::string("', '") + getComputationType() +
            std::string("_") + std::to_string(computationLabel) + std::string("')\n");
        this->setTraversed(true);
        this->setOutputTupleSetName(outputTupleSetName);
        this->setOutputColumnToApply(addedOutputColumnName);
        return ret;
    }

    int getNumInputs() override {
        return 0;
    }

    std::string getIthInputType(int i) override {
        return "";
    }

    std::string getOutputType() override {
        if (outputType == "") {
            outputType = getTypeName<OutputClass>();
        }
        return this->outputType;
    }

    bool needsMaterializeOutput() override {
        return false;
    }

    void setFollowedByLocalJoin(bool followedByLocalJoin) {
        this->followedByLocalJoin = followedByLocalJoin;
    }

    bool isFollowedByLocalJoin() {
        return followedByLocalJoin;
    }

    void setPartitionLambda (GenericLambdaObjectPtr partitionLambda) {
        this->partitionLambda = partitionLambda;
    }

protected:
    // Be careful here that we put PageCircularBufferIteratorPtr and DataProxyPtr in a pdb object.
    PageCircularBufferIteratorPtr iterator = nullptr;

    DataProxyPtr proxy = nullptr;

    String dbName;

    String setName;

    int batchSize = DEFAULT_BATCH_SIZE;

    String outputType = "";    

    bool followedByLocalJoin = false;

    int numPartitions;

    int numNodes;

    int myPartitionId;

    GenericLambdaObjectPtr partitionLambda = nullptr;
 


};

}

#endif
