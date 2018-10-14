
#ifndef SELF_LAM_H
#define SELF_LAM_H

#include "Handle.h"
#include <string>
#include "Ptr.h"
#include "TupleSet.h"
#include <vector>
#include "SimpleComputeExecutor.h"
#include "SimpleVectorPartitioner.h"
#include "TupleSetMachine.h"

namespace pdb {

template <class ClassType>
class SelfLambda : public TypedLambdaObject<Ptr<ClassType>> {

public:
    std::string inputTypeName;

public:
    // create an att access lambda; offset is the position in the input object where we are going to
    // find the input att
    SelfLambda(Handle<ClassType>& input) {
        inputTypeName = getTypeName<ClassType>();
        this->setInputIndex(0, -((input.getExactTypeInfoValue() + 1)));
    }

    std::string getTypeOfLambda() override {
        return std::string("self");
    }

    std::string typeOfAtt() {
        return inputTypeName;
    }

    std::string getInputType() {
        return inputTypeName;
    }

    unsigned int getNumInputs() override {
        return 1;
    }

    int getNumChildren() override {
        return 0;
    }

    GenericLambdaObjectPtr getChild(int which) override {
        return nullptr;
    }


    //Assumption 1: Out type must have hash function defined
    //Assumption 2: numPartitions should be multiples of numNodes
    size_t getHash(Handle<Object> input) override {

        Handle<ClassType> myIn = unsafeCast<ClassType, Object>(input);
        size_t hashVal = Hasher<ClassType>::hash(*myIn);
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
            std::cout << "to run partitioner from SelfLambda" << std::endl;
            size_t numObjects = inputObjects->size();
            for (int i = 0; i < numObjects; i++) {
               Handle<Object> myObj = (*inputObjects)[i];
               Handle<ClassType> myIn = unsafeCast<ClassType, Object>(myObj);
               size_t hashVal = Hasher<ClassType>::hash(*myIn);
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
                    std::vector<Ptr<ClassType>>* outputCol = new std::vector<Ptr<ClassType>>;
                    output->addColumn(outAtt, outputCol, true);
                }

                // get the output column
                std::vector<Ptr<ClassType>>& outColumn = output->getColumn<Ptr<ClassType>>(outAtt);

                // loop down the columns, setting the output
                int numTuples = inputColumn.size();
                outColumn.resize(numTuples);
                for (int i = 0; i < numTuples; i++) {
                    outColumn[i] = (ClassType*)&(*(inputColumn[i]));
                }

                return output;
            },
            "selfLambda");
    }
};
}

#endif
