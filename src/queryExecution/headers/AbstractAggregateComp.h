#ifndef ABSTRACT_AGG_COMP
#define ABSTRACT_AGG_COMP

#include "Computation.h"
#include "DataTypes.h"
#include "SimpleSingleTableQueryProcessor.h"
#include "DataProxy.h"
#include "PageCircularBufferIterator.h"
#include "ScanUserSet.h"
#include "DataTypes.h"
#include <vector>


namespace pdb {

/*
 * This class defines the interfaces for AggregateComp
 * This class is used in backend when type information is unknown
 */

class AbstractAggregateComp : public Computation {

public:
    // to get combiner processor
    virtual SimpleSingleTableQueryProcessorPtr getCombinerProcessor(
        std::vector<HashPartitionID> nodePartitionIds) = 0;

    // to get aggregation processor
    virtual SimpleSingleTableQueryProcessorPtr getAggregationProcessor(HashPartitionID id) = 0;

    // to get the agg out processor
    virtual SimpleSingleTableQueryProcessorPtr getAggOutProcessor() = 0;

    // to set number of partitions
    void setNumPartitions(int numPartitions) {
        this->numPartitions = numPartitions;
    }

    // to get the number of partitions
    int getNumPartitions() {
        return this->numPartitions;
    }

    // to set number of nodes
    void setNumNodes(int numNodes) {
        this->numNodes = numNodes;
    }

    // to get number of nodes
    int getNumNodes() {
        return this->numNodes;
    }

    void setBatchSize(int batchSize) override {
        this->batchSize = batchSize;
    }

    // to get the batch size
    int getBatchSize() {
        return this->batchSize;
    }

    // to set iterator for reading from output
    virtual void setIterator(PageCircularBufferIteratorPtr iterator) = 0;

    // to set proxy for reading from output
    virtual void setProxy(DataProxyPtr proxy) = 0;

    // to set database name
    virtual void setDatabaseName(std::string dbName) = 0;

    // to set the set name
    virtual void setSetName(std::string setName) = 0;


    bool needsMaterializeOutput() override {
        return this->materializeAggOut;
    }


    void setHashTable(void* hashTableLocation) {
        this->materializeAggOut = false;
        this->whereHashTableSitsForThePartition = hashTableLocation;
    }

    bool isUsingCombiner() override {
        return useCombinerOrNot;
    }

    void setUsingCombiner(bool useCombinerOrNot) override {
        this->useCombinerOrNot = useCombinerOrNot;
    }


protected:
    // number of partitions in the cluster
    int numPartitions;
    // number of nodes in the cluster
    int numNodes;
    int batchSize = 1;
    void* whereHashTableSitsForThePartition = nullptr;
    bool materializeAggOut = false;
    bool useCombinerOrNot = true;
};
}

#endif
