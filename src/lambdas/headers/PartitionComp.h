
#ifndef PARTITION_COMP
#define PARTITION_COMP

#include "Computation.h"
#include "VectorSink.h"
#include "ScanUserSet.h"
#include "TypeName.h"
#include "HashPartitionSink.h"
#include "mustache.h"

namespace pdb {

template <class KeyClass, class ValueClass>
class PartitionComp : public Computation {

public:

    /**
     * the computation returned by this method is called to perfom a transformation on the input
     * item before it is inserted into the output set to decide which partition the input item should
     * be stored.
     * @param checkMe: the input element
     * @return: the output lambda tree used to apply to the input element
     */
    virtual Lambda<KeyClass> getProjection(Handle<ValueClass> checkMe) = 0;

    // calls getProjection and getSelection to extract the lambdas
    void extractLambdas(std::map<std::string, GenericLambdaObjectPtr>& returnVal) override {
        int suffix = 0;
        Handle<ValueClass> checkMe = nullptr;
        Lambda<KeyClass> projectionLambda = getProjection(checkMe);
        projectionLambda.toMap(returnVal, suffix);
    }

    // this is a partition computation
    std::string getComputationType() override {
        return std::string("PartitionComp");
    }

    // gets the name of the i^th input type...
    std::string getIthInputType(int i) override {
        if (i == 0) {
            return getTypeName<ValueClass>();
        } else {
            return "";
        }
    }

    // get the number of inputs to this query type
    int getNumInputs() override {
        return 1;
    }

    // gets the output type of this query as a string
    std::string getOutputType() override {
        return getTypeName<ValueClass>();
    }

    // below function implements the interface for parsing computation into a TCAP string
    std::string toTCAPString(std::vector<InputTupleSetSpecifier>& inputTupleSets,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumnNames,
                             std::string& addedOutputColumnName) override {

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


    // to return Selection tcap string
    std::string toTCAPString(std::string inputTupleSetName,
                             std::vector<std::string>& inputColumnNames,
                             std::vector<std::string>& inputColumnsToApply,
                             std::vector<std::string>& childrenLambdaNames,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumnNames,
                             std::string& addedOutputColumnName,
                             std::string& myLambdaName) {
        PDB_COUT << "ABOUT TO GET TCAP STRING FOR PARTITION" << std::endl;
        Handle<ValueClass> checkMe = nullptr;
        std::string tupleSetName;
        std::vector<std::string> columnNames;
        std::string addedColumnName;
        int lambdaLabel = 0;


        std::string tcapString;

        PDB_COUT << "TO GET TCAP STRING FOR PROJECTION LAMBDA\n";
        Lambda<KeyClass> projectionLambda = getProjection(checkMe);

        tcapString += "\n/* Apply projection */\n";
        tcapString += projectionLambda.toTCAPString(inputTupleSetName,
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

       PDB_COUT << "TO REMOVE THE KEY COLUMN\n";

       mustache::data partitionCompTCAP;
       partitionCompTCAP.set("computationType", getComputationType());
       partitionCompTCAP.set("computationLabel", std::to_string(computationLabel));
       partitionCompTCAP.set("tupleSetName", tupleSetName);
       partitionCompTCAP.set("addedColumnName", addedColumnName);

       //set the output tuple set name
       mustache::mustache outputTupleSetNameTemplate{"partitionOutFor{{computationType}}{{computationLabel}}"};
       outputTupleSetName = outputTupleSetNameTemplate.render(partitionCompTCAP);

       partitionCompTCAP.set("outputTupleSetName", outputTupleSetName);

       // set the added output column
       addedOutputColumnName = inputColumnsToApply[0];

       partitionCompTCAP.set("addedOutputColumnName", addedOutputColumnName);

       tcapString += "\n/* Apply partition */\n";

       mustache::mustache partitionTCAPTemplate{"{{outputTupleSetName}} ({{addedOutputColumnName}})"
                                         " <= PARTITION ({{tupleSetName}}({{addedOutputColumnName}}, {{addedColumnName}}),"
                                         "'{{computationType}}_{{computationLabel}}')\n"};

       tcapString += partitionTCAPTemplate.render(partitionCompTCAP);

       // update the state of the computation
       outputColumnNames.clear();
       outputColumnNames.push_back(addedOutputColumnName);


       // update the state of the computation
       this->setTraversed(true);
       this->setOutputTupleSetName(outputTupleSetName);
       this->setOutputColumnToApply(addedOutputColumnName);

       // return the TCAP string
       return tcapString;

    }

    void setOutput(std::string dbName, std::string setName) override {
        this->materializeSelectionOut = true;
        this->outputSetScanner = makeObject<ScanUserSet<ValueClass>>();
        this->outputSetScanner->setDatabaseName(dbName);
        this->outputSetScanner->setSetName(setName);
    }

    void setBatchSize(int batchSize) override {
        if (this->outputSetScanner != nullptr) {
            this->outputSetScanner->setBatchSize(batchSize);
        }
    }

    // to return the database name
    std::string getDatabaseName() override {
       if (this->outputSetScanner != nullptr) {
           return this->outputSetScanner->getDatabaseName();
       } else {
          return "";
       }
    }

    // to return the set name
    std::string getSetName() override {
       if (this->outputSetScanner != nullptr) {
          return this->outputSetScanner->getSetName();
       } else {
          return "";
       }
    }

    // source for consumer to read selection output, which has been written to a user set
    ComputeSourcePtr getComputeSource(TupleSpec& outputScheme, ComputePlan& plan) override {
        if (this->outputSetScanner != nullptr) {
            std::cout << "database name is " << this->outputSetScanner->getDatabaseName() << std::endl;
            std::cout << "set name is " << this->outputSetScanner->getSetName() << std::endl;
            return outputSetScanner->getComputeSource(outputScheme, plan);
        }
        std::cout << "ERROR: get compute source for " << outputScheme << " returns nullptr" << std::endl;
        return nullptr;
    }

    // sink to write selection output
    ComputeSinkPtr getComputeSink(TupleSpec& consumeMe,
                                  TupleSpec& projection,
                                  ComputePlan& plan) override {
        return std::make_shared<HashPartitionSink<KeyClass, ValueClass>>(this->numPartitions, this->numNodes, consumeMe, projection, this->storeConflictingObjects, this->myNodeId, this->recoverData, this->nodesToRecover);

    }

    bool needsMaterializeOutput() override {
        return materializeSelectionOut;
    }

    Handle<ScanUserSet<ValueClass>>& getOutputSetScanner() {
        return outputSetScanner;
    }

    /**
     * @param storeConflictingObjects: whether to store conflicting objects
     */
    void setStoreConflictingObjects (bool storeConflictingObjects) {
        this->storeConflictingObjects = storeConflictingObjects;
    }

    /**
     * @return: whether to store conflicting objects
     */
    bool getStoreConflictingObjects () {
        return this->storeConflictingObjects;
    }



   /**
    * @param numPartitions: to set the number of partitions for this computation
    */
   void setNumPartitions (int numPartitions) {
       this->numPartitions = numPartitions;
   }

   /**
    * @return: number of partitions for this partition computation
    */
   int getNumPartitions () {
       return this->numPartitions;
   }

   /**
    * @param numNodes: to set the number of nodes for this computation
    */
   void setNumNodes (int numNodes) {
       this->numNodes = numNodes;
   }

  /**
   * @return: number of nodes for this partition computation
   */
  int getNumNodes () {
      return this->numNodes;
  }

  /**
   * @param nodeId: the id of the node that runs this computation
   */
  void setNodeId (int nodeId) {
    this->myNodeId = nodeId;
  }

  /**
   * @return: the id of the node that runs this computation
   */
  int getNodeId () {
    return this->myNodeId;
  }


  /**
   * @param recoverData: whether to recover data
   */
  void setRecoverData (bool recoverData) {
    this->recoverData = recoverData;
  }

  /**
   * @return: whether to recover data
   */
  bool getRecoverData () {
    return this->recoverData;
  }

  /**
   * @param nodesToRecover: an array of nodes to recover
   */
  void setNodesToRecover (std::vector<int> nodesToRecover) {
    this->nodesToRecover = makeObject<Vector<int>> ();
    for (size_t i = 0; i < nodesToRecover.size(); i++) {
        this->nodesToRecover->push_back(nodesToRecover[i]);
    }
  }

   /**
    * @return: a Vector of nodes to recover
    */
   Handle<Vector<int>> getNodesToRecover () {
      return this->nodesToRecover;
   }


private:
    bool materializeSelectionOut = false;
    int numPartitions = 16;
    int numNodes = 2;
    Handle<ScanUserSet<ValueClass>> outputSetScanner = nullptr;

    // below parameters should be set by the partitioner in the system code

    // whether to detect and store conflicting objects for heterogeneous replication in case of failure recovery
    bool storeConflictingObjects = false;


    // the node Id of current node
    int myNodeId = 0;

    // whether to recover data
    bool recoverData = false;

    // the Ids of nodes to recover
    Handle<Vector<int>> nodesToRecover = nullptr;
};
}

#endif
