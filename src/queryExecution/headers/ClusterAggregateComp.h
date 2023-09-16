#ifndef CLUSTER_AGG_COMP
#define CLUSTER_AGG_COMP



#include "AbstractAggregateComp.h"
#include "ScanUserSet.h"
#include "CombinerProcessor.h"
#include "AggregationProcessor.h"
#include "AggOutProcessor.h"
#include "SimpleSingleTableQueryProcessor.h"
#include "DataTypes.h"
#include "InterfaceFunctions.h"
#include "ShuffleSink.h"
#include "CombinedShuffleSink.h"
#include "MapTupleSetIterator.h"


namespace pdb {

// this aggregates items of type InputClass.  To aggregate an item, the result of getKeyProjection
// () is
// used to extract a key from on input, and the result of getValueProjection () is used to extract a
// value from an input.  Then, all values having the same key are aggregated using the += operation
// over values.
// Note that keys must have operation == as well has hash () defined.  Also, note that values must
// have the
// + operation defined.
//
// Once aggregation is completed, the key-value pairs are converted into OutputClass objects.  An
// object
// of type OutputClass must have two methods defined: KeyClass &getKey (), as well as ValueClass
// &getValue ().
// To convert a key-value pair into an OutputClass object, the result of getKey () is set to the
// desired key,
// and the result of getValue () is set to the desired value.
//


template <class OutputClass, class InputClass, class KeyClass, class ValueClass>
class ClusterAggregateComp : public AbstractAggregateComp {

public:
    // materialize aggregation output, use ScanUserSet to obtain consumer's ComputeSource
    void setOutput(std::string dbName, std::string setName) override {
        this->materializeAggOut = true;
        this->outputSetScanner = makeObject<ScanUserSet<OutputClass>>();
        this->outputSetScanner->setBatchSize(batchSize);
        this->outputSetScanner->setDatabaseName(dbName);
        this->outputSetScanner->setSetName(setName);
        this->whereHashTableSitsForThePartition = nullptr;
    }

    // set hash table pointer
    void setHashTablePointer(void* hashTablePointer) {
        this->whereHashTableSitsForThePartition = hashTablePointer;
    }

    // gets the operation tht extracts a key from an input object
    virtual Lambda<KeyClass> getKeyProjection(Handle<InputClass> aggMe) = 0;

    // gets the operation that extracts a value from an input object
    virtual Lambda<ValueClass> getValueProjection(Handle<InputClass> aggMe) = 0;

    // extract the key projection and value projection
    void extractLambdas(std::map<std::string, GenericLambdaObjectPtr>& returnVal) override {
        int suffix = 0;
        Handle<InputClass> checkMe = nullptr;
        Lambda<KeyClass> keyLambda = getKeyProjection(checkMe);
        Lambda<ValueClass> valueLambda = getValueProjection(checkMe);
        keyLambda.toMap(returnVal, suffix);
        valueLambda.toMap(returnVal, suffix);
    }


    // to populate lambdas for self learning
    void populateLambdas(long jobId, SelfLearningWrapperServer server) override {
        int suffix = 0;
        std::string compName = getComputationName();
        std::vector<std::string> computationInputTypes;
        int numInputs = getNumInputs();
        for (int i = 0; i < numInputs; i++) {
            std::string curInputType = getIthInputType(i);
            computationInputTypes.push_back(curInputType);
        }

        // we need only partitioning lambdas
        Handle<InputClass> checkMe = nullptr;
        Lambda<KeyClass> keyLambda = getKeyProjection(checkMe);
        keyLambda.toSelfLearningDatabase(jobId, compName, computationInputTypes,
               server, suffix);

    }



    // sink for aggregation producing phase output, shuffle data will be combined from the sink
    ComputeSinkPtr getComputeSink(TupleSpec& consumeMe,
                                  TupleSpec& projection,
                                  ComputePlan& plan) override {

        if (this->isUsingCombiner() == true) {
            return std::make_shared<ShuffleSink<KeyClass, ValueClass>>(
                numPartitions, consumeMe, projection);
        } else {
            if (numNodes == 0) {
                std::cout << "ERROR: cluster has 0 node" << std::endl;
                return nullptr;
            }
            if (numPartitions < numNodes) {
                std::cout << "ERROR: each node must have at least one partition" << std::endl;
                return nullptr;
            }
            return std::make_shared<CombinedShuffleSink<KeyClass, ValueClass>>(
                numPartitions / numNodes, numNodes, consumeMe, projection);
        }
    }

    // source for consumer to read aggregation results, aggregation results are written to user set
    ComputeSourcePtr getComputeSource(TupleSpec& outputScheme, ComputePlan& plan) override {
        // materialize aggregation result to user set
        if (this->materializeAggOut == true) {
            if (outputSetScanner != nullptr) {
                return outputSetScanner->getComputeSource(outputScheme, plan);
            }
            return nullptr;

            // not materialize aggregation result, keep them in hash table
        } else {
            if (whereHashTableSitsForThePartition != nullptr) {
                Handle<Object> myHashTable =
                    ((Record<Object>*)whereHashTableSitsForThePartition)->getRootObject();
                return std::make_shared<MapTupleSetIterator<KeyClass, ValueClass, OutputClass>>(
                    myHashTable, batchSize);
            }
            return nullptr;
        }
    }

    // to return processor for combining data written to shuffle sink
    // the combiner processor is used in the end of aggregation producing phase
    // the input is data written to shuffle sink
    // the output is data for shuffling
    SimpleSingleTableQueryProcessorPtr getCombinerProcessor(
        std::vector<HashPartitionID> partitions) override {
        return make_shared<CombinerProcessor<KeyClass, ValueClass>>(partitions);
    }

    // to return processor for aggregating on shuffle data
    // the aggregation processor is used in the  aggregation consuming phase
    // the input is shuffle data
    // the output are intermediate pages of arbitrary size allocated on heap
    SimpleSingleTableQueryProcessorPtr getAggregationProcessor(HashPartitionID id) override {
        return make_shared<AggregationProcessor<KeyClass, ValueClass>>(id);
    }

    // to return processor for writing aggregation results to a user set
    // the agg out processor is used in the aggregation consuming phase for materializing
    // aggregation results to user set
    // the input is the output of aggregation processor
    // the output is written to a user set
    SimpleSingleTableQueryProcessorPtr getAggOutProcessor() override {
        return make_shared<AggOutProcessor<OutputClass, KeyClass, ValueClass>>();
    }

    // to set iterator for scanning the materialized aggregation output that is stored in a user set
    void setIterator(PageCircularBufferIteratorPtr iterator) override {
        this->outputSetScanner->setIterator(iterator);
    }

    // to set data proxy for scanning the materialized aggregation output that is stored in a user
    // set
    void setProxy(DataProxyPtr proxy) override {
        this->outputSetScanner->setProxy(proxy);
    }

    // to set the database name
    void setDatabaseName(std::string dbName) override {
        this->outputSetScanner->setDatabaseName(dbName);
    }

    // to set the set name
    void setSetName(std::string setName) override {
        this->outputSetScanner->setSetName(setName);
    }

    // to return the database name
    std::string getDatabaseName() override {
        return this->outputSetScanner->getDatabaseName();
    }

    // to return the set name
    std::string getSetName() override {
        return this->outputSetScanner->getSetName();
    }

    // this is an aggregation comp
    std::string getComputationType() override {
        return std::string("ClusterAggregationComp");
    }

    // to get output type
    std::string getOutputType() override {
        return getTypeName<OutputClass>();
    }

    // get the number of inputs to this query type
    int getNumInputs() override {
        return 1;
    }

    // get the name of the i^th input type...
    std::string getIthInputType(int i) override {
        if (i == 0) {
            return getTypeName<InputClass>();
        } else {
            return "";
        }
    }

    // below function implements the interface for parsing computation into a TCAP string
    std::string toTCAPString(std::vector<InputTupleSetSpecifier>& inputTupleSets,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumnNames,
                             std::string& addedOutputColumnName) override {

        this->setComputationName(getComputationType()+"_"+std::to_string(computationLabel));
        if (inputTupleSets.size() == 0) {
            return "";
        }
        InputTupleSetSpecifier inputTupleSet = inputTupleSets[0];
        std::vector<std::string> childrenLambdaNames;
        std::string myLambdaName;
        return toTCAPString(inputTupleSet.getTupleSetName(),
                            inputTupleSet.getColumnNamesToKeep(),
                            inputTupleSet.getColumnNamesToApply(),
                            childrenLambdaNames,
                            computationLabel,
                            outputTupleSetName,
                            outputColumnNames,
                            addedOutputColumnName,
                            myLambdaName);
    }


    // to return Aggregate tcap string
    std::string toTCAPString(std::string inputTupleSetName,
                             std::vector<std::string>& inputColumnNames,
                             std::vector<std::string>& inputColumnsToApply,
                             std::vector<std::string>& childrenLambdaNames,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumnNames,
                             std::string& addedOutputColumnName,
                             std::string& myLambdaName) {
        std::string tcapString = "";
        Handle<InputClass> checkMe = nullptr;
        Lambda<KeyClass> keyLambda = getKeyProjection(checkMe);
        std::string tupleSetName;
        std::vector<std::string> columnNames;
        std::string addedColumnName;
        int lambdaLabel = 0;

        std::vector<std::string> columnsToApply;
        for (int i = 0; i < inputColumnsToApply.size(); i++) {
            columnsToApply.push_back(inputColumnsToApply[i]);
        }
        tcapString += "\n/* Extract key for aggregation */\n";
        tcapString += keyLambda.toTCAPString(inputTupleSetName,
                                             inputColumnNames,
                                             inputColumnsToApply,
                                             childrenLambdaNames,
                                             lambdaLabel,
                                             getComputationType(),
                                             computationLabel,
                                             tupleSetName,
                                             columnNames,
                                             addedColumnName,
                                             myLambdaName,
                                             false);
        Lambda<ValueClass> valueLambda = getValueProjection(checkMe);
        std::vector<std::string> columnsToKeep;
        columnsToKeep.push_back(addedColumnName);


        tcapString += "\n/* Extract value for aggregation */\n";
        tcapString += valueLambda.toTCAPString(tupleSetName,
                                               columnsToKeep,
                                               columnsToApply,
                                               childrenLambdaNames,
                                               lambdaLabel,
                                               getComputationType(),
                                               computationLabel,
                                               outputTupleSetName,
                                               outputColumnNames,
                                               addedOutputColumnName,
                                               myLambdaName,
                                               false);
        std::string newTupleSetName =
            "aggOutFor" + getComputationType() + "_" + std::to_string(computationLabel);
        tcapString += "\n/* Apply aggregation */\n";
        std::string addedOutputColumnName1 = "aggOutFor" + std::to_string(computationLabel);
        tcapString += newTupleSetName + "(" + addedOutputColumnName1 + ") <= AGGREGATE (" +
            outputTupleSetName + " (" + addedColumnName + ", " + addedOutputColumnName + "), '";
        tcapString += getComputationType() + "_" + std::to_string(computationLabel) + "')\n";
        outputTupleSetName = newTupleSetName;
        outputColumnNames.clear();
        outputColumnNames.push_back(addedOutputColumnName1);
        this->setTraversed(true);
        this->setOutputTupleSetName(outputTupleSetName);
        this->setOutputColumnToApply(addedOutputColumnName1);
        addedOutputColumnName = addedOutputColumnName1;
        return tcapString;
    }

    Handle<ScanUserSet<OutputClass>>& getOutputSetScanner() {
        return outputSetScanner;
    }

    void setCollectAsMap(bool collectAsMapOrNot) override {
        this->collectAsMapOrNot = collectAsMapOrNot;
    }

    bool isCollectAsMap() override {
        return this->collectAsMapOrNot;
    }

    int getNumNodesToCollect() override {
        return this->numNodesToCollect;
    }

    void setNumNodesToCollect(int numNodesToCollect) override {
        this->numNodesToCollect = numNodesToCollect;
    }

protected:
    Handle<ScanUserSet<OutputClass>> outputSetScanner = nullptr;
    bool collectAsMapOrNot = false;
    int numNodesToCollect = 1;
};

}

#endif
