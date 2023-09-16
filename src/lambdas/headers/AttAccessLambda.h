
#ifndef ATT_ACCESS_LAM_H
#define ATT_ACCESS_LAM_H

#include "Handle.h"
#include <string>
#include "Ptr.h"
#include "TupleSet.h"
#include <vector>
#include "SimpleComputeExecutor.h"
#include "SimplePartitioner.h"
#include "SimpleVectorPartitioner.h"
#include "TupleSetMachine.h"
#include "DataTypes.h"

namespace pdb {

template <class Out, class ClassType>
class AttAccessLambda : public TypedLambdaObject<Ptr<Out>> {

public:
    size_t offsetOfAttToProcess;
    std::string inputTypeName;
    std::string attName;
    std::string attTypeName;

public:
    // create an att access lambda; offset is the position in the input object where we are going to
    // find the input att
    AttAccessLambda(std::string inputTypeNameIn,
                    std::string attNameIn,
                    std::string attType,
                    Handle<ClassType>& input,
                    size_t offset)
        : offsetOfAttToProcess(offset),
          inputTypeName(inputTypeNameIn),
          attName(attNameIn),
          attTypeName(attType) {

        this->setInputIndex(0, -(input.getExactTypeInfoValue() + 1));
    }

    std::string getTypeOfLambda() override {
        return std::string("attAccess");
    }

    std::string getIdentifierOfLambda() override {
        return attName;
    }


    std::string typeOfAtt() {
        return attTypeName;
    }

    std::string whichAttWeProcess() {
        return attName;
    }

    unsigned int getNumInputs() override {
        return 1;
    }


    std::string getInputType() {
        return inputTypeName;
    }

    std::string toTCAPString(std::vector<std::string>& inputTupleSetNames,
                             std::vector<std::string>& inputColumnNames,
                             std::vector<std::string>& inputColumnsToApply,
                             std::vector<std::string>& childrenLambdaNames,
                             int lambdaLabel,
                             std::string computationName,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumns,
                             std::string& outputColumnName,
                             std::string& myLambdaName,
                             MultiInputsBase* multiInputsComp = nullptr,
                             bool amIPartOfJoinPredicate = false,
                             bool amILeftChildOfEqualLambda = false,
                             bool amIRightChildOfEqualLambda = false,
                             std::string parentLambdaName = "",
                             bool isSelfJoin = false) override {
        std::string tcapString = "";

        std::string inputTupleSetName = inputTupleSetNames[0];

        std::string tupleSetMidTag = "";
        std::string computationNameWithLabel =
            computationName + "_" + std::to_string(computationLabel);
        myLambdaName = getTypeOfLambda() + "_" + std::to_string(lambdaLabel);
        std::string originalInputColumnToApply;
        int myIndex;
        if (multiInputsComp == nullptr) {
            tupleSetMidTag = "OutFor";
        } else {
            tupleSetMidTag = "ExtractedFor";
            myIndex = this->getInputIndex(0);
            PDB_COUT << myLambdaName << ": myIndex=" << myIndex << std::endl;
            inputTupleSetName = multiInputsComp->getTupleSetNameForIthInput(myIndex);
            PDB_COUT << "inputTupleSetName=" << inputTupleSetName << std::endl;
            inputColumnNames = multiInputsComp->getInputColumnsForIthInput(myIndex);
            inputColumnsToApply.clear();
            inputColumnsToApply.push_back(multiInputsComp->getNameForIthInput(myIndex));
            originalInputColumnToApply = multiInputsComp->getNameForIthInput(myIndex);
            PDB_COUT << "originalInputColumnToApply=" << originalInputColumnToApply << std::endl;
        }


        outputTupleSetName = "attAccess_" + std::to_string(lambdaLabel) + tupleSetMidTag +
            computationName + std::to_string(computationLabel);

        outputColumnName = "att_" + std::to_string(lambdaLabel) + tupleSetMidTag + "_" + attName;
        outputColumns.clear();
        for (unsigned int i = 0; i < inputColumnNames.size(); i++) {
            outputColumns.push_back(inputColumnNames[i]);
        }
        outputColumns.push_back(outputColumnName);

        tcapString += this->getTCAPString(inputTupleSetName,
                                          inputColumnNames,
                                          inputColumnsToApply,
                                          outputTupleSetName,
                                          outputColumns,
                                          outputColumnName,
                                          "APPLY",
                                          computationNameWithLabel,
                                          myLambdaName);

        if (multiInputsComp != nullptr) {
            if (amILeftChildOfEqualLambda || amIRightChildOfEqualLambda) {
                inputTupleSetName = outputTupleSetName;
                inputColumnNames.clear();
                for (unsigned int i = 0; i < outputColumns.size(); i++) {
                    // we want to remove the extracted value column from here
                    if (outputColumns[i] != outputColumnName) {
                        inputColumnNames.push_back(outputColumns[i]);
                    }
                }
                inputColumnsToApply.clear();
                inputColumnsToApply.push_back(outputColumnName);

                std::string hashOperator = "";
                if (amILeftChildOfEqualLambda == true) {
                    hashOperator = "HASHLEFT";
                } else {
                    hashOperator = "HASHRIGHT";
                }
                outputTupleSetName = outputTupleSetName + "_hashed";
                outputColumnName = outputColumnName + "_hash";
                outputColumns.clear();

                for (unsigned int i = 0; i < inputColumnNames.size(); i++) {
                    outputColumns.push_back(inputColumnNames[i]);
                }
                outputColumns.push_back(outputColumnName);

                tcapString += this->getTCAPString(inputTupleSetName,
                                                  inputColumnNames,
                                                  inputColumnsToApply,
                                                  outputTupleSetName,
                                                  outputColumns,
                                                  outputColumnName,
                                                  hashOperator,
                                                  computationNameWithLabel,
                                                  parentLambdaName);
            }

            if (isSelfJoin == false) {
                for (unsigned int index = 0; index < multiInputsComp->getNumInputs(); index++) {
                    std::string curInput = multiInputsComp->getNameForIthInput(index);
                    auto iter = std::find(outputColumns.begin(), outputColumns.end(), curInput);
                    if (iter != outputColumns.end()) {
                        multiInputsComp->setTupleSetNameForIthInput(index, outputTupleSetName);
                        multiInputsComp->setInputColumnsForIthInput(index, outputColumns);
                        multiInputsComp->setInputColumnsToApplyForIthInput(index, outputColumnName);


                    }
                    if (originalInputColumnToApply == curInput) {
                        multiInputsComp->setTupleSetNameForIthInput(index, outputTupleSetName);
                        multiInputsComp->setInputColumnsForIthInput(index, outputColumns);
                        multiInputsComp->setInputColumnsToApplyForIthInput(index, outputColumnName);


                    }
                }
            } else {
                // only update myIndex
                multiInputsComp->setTupleSetNameForIthInput(myIndex, outputTupleSetName);
                multiInputsComp->setInputColumnsForIthInput(myIndex, outputColumns);
                multiInputsComp->setInputColumnsToApplyForIthInput(myIndex, outputColumnName);
            }
        }
        return tcapString;
    }

    int getNumChildren() override {
        return 0;
    }

    GenericLambdaObjectPtr getChild(int which) override {
        return nullptr;
    }

    size_t getHash(Handle<Object> input) override {

        Handle<ClassType> myIn = unsafeCast<ClassType, Object>(input);
        Ptr<Out> myOut =  (Out*)((char*)&(*(myIn)) + offsetOfAttToProcess);
        Out myValue = *myOut;
        size_t hashVal = Hasher<Out>::hash(myValue);
        return hashVal;
    }        


    //Assumption 1: Out type must have hash function defined
    //Assumption 2: numPartitions should be multiples of numNodes
    SimpleVectorPartitionerPtr getPartitioner() override {

        return std::make_shared<SimpleVectorPartitioner> (
           [=](int numNodes,
               int numPartitions,
               Handle<Vector<Handle<Object>>> & inputObjects,
               std::shared_ptr<std::unordered_map<NodeID, Handle<Vector<Handle<Object>>>>> partitionedObjects) {
            size_t numObjects = inputObjects->size();
            for (size_t i = 0; i < numObjects; i++) {
               Handle<Object> myObj = (*inputObjects)[i];
               Handle<ClassType> myIn = unsafeCast<ClassType, Object>(myObj);
               Ptr<Out> myOut = (Out*)((char*)&(*(myIn)) + offsetOfAttToProcess);
               Out myValue = *myOut;
               size_t hashVal = Hasher<Out>::hash(myValue);
               NodeID nodeId;
               unsigned int partitionId;
#ifndef NO_MOD_PARTITION
               partitionId = hashVal % numPartitions;
#else
               partitionId = (hashVal / numPartitions) % numPartitions;
#endif
               nodeId = (partitionId /(numPartitions / numNodes));
               Handle<Vector<Handle<Object>>> myVec = (*partitionedObjects)[nodeId];
               if(myVec == nullptr) {
                   myVec = makeObject<Vector<Handle<Object>>>();
                   (*partitionedObjects)[nodeId] = myVec;
               }
               myVec->push_back(myObj);
            
            }
            return true;
        }); 

    }


    //Assumption 1: Out type must have hash function defined
    //Assumption 2: numPartitions should be multiples of numNodes
    //std::function<NodeID(Handle<Object> &, int numNodes, int numPartitions)> partitionFunc
    SimplePartitionerPtr getObjectPartitioner() override {

        return std::make_shared<SimplePartitioner> (
           [=](Handle<Object>& myObj,
               int numNodes,
               int numPartitions) {
               Handle<ClassType> myIn = unsafeCast<ClassType, Object>(myObj);
               Ptr<Out> myOut = (Out*)((char*)&(*(myIn)) + offsetOfAttToProcess);
               Out myValue = *myOut;
               size_t hashVal = Hasher<Out>::hash(myValue);
               NodeID nodeId;
               unsigned int partitionId;
#ifndef NO_MOD_PARTITION
               partitionId = hashVal % numPartitions;
#else
               partitionId = (hashVal / numPartitions) % numPartitions;
#endif
               nodeId = (partitionId /(numPartitions / numNodes));
               return nodeId;
        });

    }




    ComputeExecutorPtr getExecutor(TupleSpec& inputSchema,
                                   TupleSpec& attsToOperateOn,
                                   TupleSpec& attsToIncludeInOutput) override {

        // create the output tuple set
        TupleSetPtr output = std::make_shared<TupleSet>();

        // create the machine that is going to setup the output tuple set, using the input tuple set
        TupleSetSetupMachinePtr myMachine =
            std::make_shared<TupleSetSetupMachine>(inputSchema, attsToIncludeInOutput);

        // this is the input attribute that we will process
        std::vector<int> matches = myMachine->match(attsToOperateOn);
        int whichAtt = matches[0];

        // this is the output attribute
        int outAtt = attsToIncludeInOutput.getAtts().size();

        return std::make_shared<SimpleComputeExecutor>(
            output,
            [=](TupleSetPtr input) {
                // set up the output tuple set
                myMachine->setup(input, output);

                // get the columns to operate on
                std::vector<Handle<ClassType>>& inputColumn =
                    input->getColumn<Handle<ClassType>>(whichAtt);

                // setup the output column, if it is not already set up
                if (!output->hasColumn(outAtt)) {
                    std::vector<Ptr<Out>>* outputCol = new std::vector<Ptr<Out>>;
                    if (outputCol == nullptr) {
                        std::cout << "AttAccessLambda: Failed to allocate memory" << std::endl;
                        exit(1);
                    }
                    output->addColumn(outAtt, outputCol, true);
                }

                // get the output column
                std::vector<Ptr<Out>>& outColumn = output->getColumn<Ptr<Out>>(outAtt);

                // loop down the columns, setting the output
                int numTuples = inputColumn.size();
                outColumn.resize(numTuples);
                for (int i = 0; i < numTuples; i++) {
                    outColumn[i] = (Out*)((char*)&(*(inputColumn[i])) + offsetOfAttToProcess);
                }

                return output;
            },
            "attAccessLambda");
    }
};
}

#endif
