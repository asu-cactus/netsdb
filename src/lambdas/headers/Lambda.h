
#ifndef LAMBDA_H
#define LAMBDA_H

#include <memory>
#include <vector>
#include <functional>
#include "SelfLearningWrapperServer.h"
#include "Object.h"
#include "Handle.h"
#include "Ptr.h"
#include "TupleSpec.h"
#include "ComputeExecutor.h"
#include "LambdaHelperClasses.h"
#include "DereferenceLambda.h"
#include "MultiInputsBase.h"

namespace pdb {

template <class ReturnType>
class Lambda {

private:
    // in case we wrap up a non-pointer type
    std::shared_ptr<TypedLambdaObject<ReturnType>> tree;

    // does the actual tree traversal
    // JiaNote: I changed below method from pre-order traversing to post-order traversing, so that
    // it follows the lambda execution ordering
    static void traverse(std::map<std::string, GenericLambdaObjectPtr>& fillMe,
                         GenericLambdaObjectPtr root,
                         int& startLabel) {

        for (int i = 0; i < root->getNumChildren(); i++) {
            GenericLambdaObjectPtr child = root->getChild(i);
            traverse(fillMe, child, startLabel);
        }
        std::string myName = root->getTypeOfLambda();
        myName = myName + "_" + std::to_string(startLabel);
        startLabel++;
        fillMe[myName] = root;
    }


    //JiaNote: another traverse function to populate lambda to selfLearning database
    static void traverseForSelfLearning (long& jobId,
                                         std::string& computationName,
                                         std::vector<std::string>& computationInputTypes,
                                         SelfLearningWrapperServer& server,
                                         GenericLambdaObjectPtr root,
                                         int& startLabel) {
        for (int i = 0; i < root->getNumChildren(); i++) {
            GenericLambdaObjectPtr child = root->getChild(i);
            traverseForSelfLearning(jobId, computationName, computationInputTypes, server, child, startLabel);
        }
        //now we only create lambda record for lambdas that has one input and one output
        if ((root->getNumChildren() == 0)&&(root->getNumInputs() == 1)) {
             
             std::string lambdaName = root->getTypeOfLambda() + "_" + std::to_string(startLabel);
             std::string outputType = root->getOutputType();
             int outputTypeId = VTableMap::getIDByName(outputType);
             unsigned int numInputs = root->getNumInputs();
             std::string inputTypes = "";
             int firstInputTypeId = 8191;
             unsigned int indexInComputationInputs = root->getInputIndex(0);
             if(computationName.find("Aggregation") != string::npos) {
                 indexInComputationInputs = 0; 
             }
             std::string curTypeName = computationInputTypes[indexInComputationInputs];
             //add an entry for this lambda to self learning database
             long id;
             server.createLambda(jobId, 
                                 root->getTypeOfLambda(), 
                                 root->getIdentifierOfLambda(), 
                                 computationName, 
                                 lambdaName, 
                                 indexInComputationInputs, 
                                 curTypeName, 
                                 outputType, 
                                 id);
        }
        startLabel++;
    }


    void getInputs(std::vector<std::string>& allInputs,
                   GenericLambdaObjectPtr root,
                   MultiInputsBase* multiInputsBase) {
        for (int i = 0; i < root->getNumChildren(); i++) {
            GenericLambdaObjectPtr child = root->getChild(i);
            getInputs(allInputs, child, multiInputsBase);
        }
        if (root->getNumChildren() == 0) {
            for (int i = 0; i < root->getNumInputs(); i++) {
                std::string myName = multiInputsBase->getNameForIthInput(root->getInputIndex(i));
                auto iter = std::find(allInputs.begin(), allInputs.end(), myName);
                if (iter == allInputs.end()) {
                    allInputs.push_back(myName);
                }
            }
        }
    }


    // JiaNote: below function is to generate a sequence of TCAP Strings for this Lambda tree
    static void getTCAPString(std::vector<std::string>& tcapStrings,
                              std::vector<std::string>& inputTupleSetNames,
                              std::vector<std::string>& inputColumnNames,
                              std::vector<std::string>& inputColumnsToApply,
                              std::vector<std::string>& childrenLambdaNames,
                              GenericLambdaObjectPtr root,
                              int& lambdaLabel,
                              std::string computationName,
                              int computationLabel,
                              std::string& addedOutputColumnName,
                              std::string& myLambdaName,
                              std::string& outputTupleSetName,
                              MultiInputsBase* multiInputsComp = nullptr,
                              bool amIPartOfJoinPredicate = false,
                              bool amILeftChildOfEqualLambda = false,
                              bool amIRightChildOfEqualLambda = false,
                              std::string parentLambdaName = "",
                              bool isSelfJoin = false) {
        std::vector<std::string> columnsToApply;
        std::vector<std::string> childrenLambdas;
        std::vector<std::string> inputNames;
        std::vector<std::string> inputColumns;
        if (root->getNumChildren() > 0) {
            for (int i = 0; i < inputColumnsToApply.size(); i++) {
                columnsToApply.push_back(inputColumnsToApply[i]);
            }
            inputColumnsToApply.clear();
            for (int i = 0; i < childrenLambdaNames.size(); i++) {
                childrenLambdas.push_back(childrenLambdaNames[i]);
            }
            childrenLambdaNames.clear();
            for (int i = 0; i < inputTupleSetNames.size(); i++) {
                auto iter = std::find(inputNames.begin(), inputNames.end(), inputTupleSetNames[i]);
                if (iter == inputNames.end()) {
                    inputNames.push_back(inputTupleSetNames[i]);
                }
            }
            inputTupleSetNames.clear();
            for (int i = 0; i < inputColumnNames.size(); i++) {
                inputColumns.push_back(inputColumnNames[i]);
            }
            inputColumnNames.clear();
        }


        std::string myTypeName = root->getTypeOfLambda();
        PDB_COUT << "\tExtracted lambda named: " << myTypeName << "\n";
        std::string myName =
            myTypeName + "_" + std::to_string(lambdaLabel + root->getNumChildren());

        bool isLeftChildOfEqualLambda = false;
        bool isRightChildOfEqualLambda = false;
        bool isChildSelfJoin = false;
        GenericLambdaObjectPtr nextChild = nullptr;
        for (int i = 0; i < root->getNumChildren(); i++) {
            GenericLambdaObjectPtr child = root->getChild(i);
            if ((i + 1) < root->getNumChildren()) {
                nextChild = root->getChild(i + 1);
            }
            if (myTypeName == "==") {
                if (i == 0) {
                    isLeftChildOfEqualLambda = true;
                }
                if (i == 1) {
                    isRightChildOfEqualLambda = true;
                }
            }
            if (((isLeftChildOfEqualLambda == true) || (isRightChildOfEqualLambda == true)) &&
                (multiInputsComp != nullptr)) {

                std::string nextInputName = "";
                if (nextChild != nullptr) {
                    nextInputName =
                        multiInputsComp->getNameForIthInput(nextChild->getInputIndex(0));
                }
                std::string myInputName =
                    multiInputsComp->getNameForIthInput(child->getInputIndex(0));
                if (nextInputName == myInputName) {
                    isChildSelfJoin = true;
                }
            }
            getTCAPString(tcapStrings,
                          inputNames,
                          inputColumns,
                          columnsToApply,
                          childrenLambdas,
                          child,
                          lambdaLabel,
                          computationName,
                          computationLabel,
                          addedOutputColumnName,
                          myLambdaName,
                          outputTupleSetName,
                          multiInputsComp,
                          amIPartOfJoinPredicate,
                          isLeftChildOfEqualLambda,
                          isRightChildOfEqualLambda,
                          myName,
                          isChildSelfJoin);
            inputColumnsToApply.push_back(addedOutputColumnName);
            childrenLambdaNames.push_back(myLambdaName);
            if (multiInputsComp != nullptr) {
                auto iter = std::find(
                    inputTupleSetNames.begin(), inputTupleSetNames.end(), outputTupleSetName);
                if (iter == inputTupleSetNames.end()) {
                    inputTupleSetNames.push_back(outputTupleSetName);
                }
            } else {
                inputTupleSetNames.clear();
                inputTupleSetNames.push_back(outputTupleSetName);
                inputColumnNames.clear();
            }
            for (int j = 0; j < inputColumns.size(); j++) {
                auto iter =
                    std::find(inputColumnNames.begin(), inputColumnNames.end(), inputColumns[j]);
                if (iter == inputColumnNames.end()) {
                    inputColumnNames.push_back(inputColumns[j]);
                }
            }
            isLeftChildOfEqualLambda = false;
            isRightChildOfEqualLambda = false;
            isChildSelfJoin = false;
            nextChild = nullptr;
        }
        std::vector<std::string> outputColumns;
        std::string tcapString = root->toTCAPString(inputTupleSetNames,
                                                    inputColumnNames,
                                                    inputColumnsToApply,
                                                    childrenLambdaNames,
                                                    lambdaLabel,
                                                    computationName,
                                                    computationLabel,
                                                    outputTupleSetName,
                                                    outputColumns,
                                                    addedOutputColumnName,
                                                    myLambdaName,
                                                    multiInputsComp,
                                                    amIPartOfJoinPredicate,
                                                    amILeftChildOfEqualLambda,
                                                    amIRightChildOfEqualLambda,
                                                    parentLambdaName,
                                                    isSelfJoin);
        tcapStrings.push_back(tcapString);
        lambdaLabel++;
        if (multiInputsComp == nullptr) {
            inputTupleSetNames.clear();
            inputTupleSetNames.push_back(outputTupleSetName);
        }
        inputColumnNames.clear();
        for (int i = 0; i < outputColumns.size(); i++) {
            inputColumnNames.push_back(outputColumns[i]);
        }
    }

public:
    unsigned int getInputIndex() {
        return tree->getInputIndex();
    }

    // create a lambda tree that returns a pointer
    Lambda(LambdaTree<Ptr<ReturnType>> treeWithPointer) {

        // a problem is that consumers of this lambda will not be able to deal with a
        // Ptr<ReturnType>...
        // so we need to add an additional operation that dereferences the pointer
        std::shared_ptr<DereferenceLambda<ReturnType>> newRoot =
            std::make_shared<DereferenceLambda<ReturnType>>(treeWithPointer);
        tree = newRoot;
    }

    // create a lambda tree that returns a non-pointer
    Lambda(LambdaTree<ReturnType> tree) : tree(tree.getPtr()) {}

    // convert one of these guys to a map
    void toMap(std::map<std::string, GenericLambdaObjectPtr>& returnVal, int& suffix) {
        traverse(returnVal, tree, suffix);
    }

    //populate a lambda tree to the selfLearning database
    void toSelfLearningDatabase(long& jobId,
                                std::string& computationName,
                                std::vector<std::string>& computationInputTypes,
                                SelfLearningWrapperServer& server,
                                int& startLabel 
                                ) {
        traverseForSelfLearning(jobId, computationName, computationInputTypes, server,
                                tree, startLabel); 

    }


    std::vector<std::string> getAllInputs(MultiInputsBase* multiInputsBase) {
        std::vector<std::string> ret;
        this->getInputs(ret, tree, multiInputsBase);
        return ret;
    }

    // to get the TCAPString for this lambda tree
    std::string toTCAPString(std::string inputTupleSetName,
                             std::vector<std::string>& inputColumnNames,
                             std::vector<std::string>& inputColumnsToApply,
                             std::vector<std::string>& childrenLambdaNames,
                             int& lambdaLabel,
                             std::string computationName,
                             int computationLabel,
                             std::string& outputTupleSetName,
                             std::vector<std::string>& outputColumnNames,
                             std::string& addedOutputColumnName,
                             std::string& myLambdaName,
                             bool whetherToRemoveUnusedOutputColumns,
                             MultiInputsBase* multiInputsComp = nullptr,
                             bool amIPartOfJoinPredicate = false) {
        std::vector<std::string> tcapStrings;
        std::string outputTCAPString;
        std::vector<std::string> inputTupleSetNames;
        inputTupleSetNames.push_back(inputTupleSetName);
        std::vector<std::string> columnNames;
        for (int i = 0; i < inputColumnNames.size(); i++) {
            columnNames.push_back(inputColumnNames[i]);
        }
        std::vector<std::string> columnsToApply;
        for (int i = 0; i < inputColumnsToApply.size(); i++) {
            columnsToApply.push_back(inputColumnsToApply[i]);
        }
        std::vector<std::string> childrenLambdas;
        for (int i = 0; i < childrenLambdaNames.size(); i++) {
            childrenLambdas.push_back(childrenLambdaNames[i]);
        }
        getTCAPString(tcapStrings,
                      inputTupleSetNames,
                      columnNames,
                      columnsToApply,
                      childrenLambdas,
                      tree,
                      lambdaLabel,
                      computationName,
                      computationLabel,
                      addedOutputColumnName,
                      myLambdaName,
                      outputTupleSetName,
                      multiInputsComp,
                      amIPartOfJoinPredicate);
        PDB_COUT << "Lambda: lambdaLabel=" << lambdaLabel << std::endl;
        bool isOutputInInput = false;
        outputColumnNames.clear();
        if (whetherToRemoveUnusedOutputColumns == false) {
            for (int i = 0; i < columnNames.size(); i++) {
                outputColumnNames.push_back(columnNames[i]);
                if (addedOutputColumnName == columnNames[i]) {
                    isOutputInInput = true;
                }
            }
            if (isOutputInInput == false) {
                outputColumnNames.push_back(addedOutputColumnName);
            }
        } else {
            outputColumnNames.push_back(addedOutputColumnName);
        }

        if (whetherToRemoveUnusedOutputColumns == true) {
            int last = tcapStrings.size() - 1;
            PDB_COUT << "tcapStrings[" << last << "]=" << tcapStrings[last] << std::endl;
            std::string right = tcapStrings[last].substr(tcapStrings[last].find("<="));
            int pos1 = right.find_last_of("(");
            int pos2 = right.rfind("),");
            right.replace(pos1 + 1, pos2 - 1 - (pos1 + 1) + 1, "");
            tcapStrings[last] = outputTupleSetName + " (" + addedOutputColumnName + ") " + right;
        }
        for (int i = 0; i < tcapStrings.size(); i++) {
            outputTCAPString = outputTCAPString + tcapStrings[i];
        }
        return outputTCAPString;
    }
};
}

#endif
