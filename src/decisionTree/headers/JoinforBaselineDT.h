#ifndef BASELINE_JOIN_DT_H
#define BASELINE_JOIN_DT_H

#pragma once

#include <JoinComp.h>
#include <LambdaCreationFunctions.h>
#include "TreeNode.h"
#include "Tree.h"
#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"

namespace decisiontree{
	
class JoinforBaselineDT : public JoinComp<FFMatrixBlock, Tree, FFMatrixBlock> {

 public:

  ENABLE_DEEP_COPY

  JoinforBaselineDT() = default;

  Lambda<bool> getSelection(Handle<Tree> in1, Handle<FFMatrixBlock> in2) override {
  	return makeLambda(in1, in2, [](Handle<Tree>& in1, Handle<FFMatrixBlock>& in2) { return true; });
  }

  Lambda<Handle<FFMatrixBlock>> getProjection(Handle<Tree> in1, Handle<FFMatrixBlock> in2) override {
    return makeLambda(in1, in2, [](Handle<Tree>& in1, Handle<FFMatrixBlock>& in2) {
      // get the input features matrix information
    	uint32_t inNumRow = in2->getRowNums();
    	uint32_t inNumCol = in2->getColNums();
    	uint32_t inBlockRowIndex = in2->getBlockRowIndex();
    	uint32_t inBlockColIndex = in2->getBlockColIndex();
    	
    	// testing purpose
    	std::cout << "Finish load the metadata" << std::endl;
    	std::cout << inNumRow << "," << inNumCol << std::endl;
    	std::cout << inBlockRowIndex << "," << inBlockColIndex << std::endl;

      // get the input features
    	double *inData = in2->getValue().rawData->c_ptr();
      // get the decision tree
      pdb::Vector<pdb::Handle<decisiontree::Node>> tree = in1->get_tree();

      // set the output matrix
    	pdb::Handle<pdb::Vector<double>> resultMatrix = pdb::makeObject<pdb::Vector<double>>();

      // set the node of the tree
      decisiontree::Node * treeNode = nullptr;

      // set a new vetor to store the whole tree
      std::vector<decisiontree::Node> vectNode;
      for(int i = 0; i < tree.size(); i++){
        pdb::Handle<decisiontree::Node> thisNodePtr = tree[i];
        decisiontree::Node thisNode = decisiontree::Node(thisNodePtr->nodeID,thisNodePtr->indexID,thisNodePtr->isLeaf,thisNodePtr->leftChild,thisNodePtr->rightChild,thisNodePtr->returnClass);
        vectNode.push_back(thisNode);
      }

      // inference
    	for (int i = 0; i < inNumRow; i++){
        // pass the root node of the tree
        treeNode = & vectNode.at(0);
        while(treeNode->isLeaf == false){
          double inputValue = inData[i*inNumCol+treeNode->indexID];
          if(inputValue <= treeNode->returnClass){
            * treeNode = * (treeNode + (treeNode->leftChild));
          }else{
            * treeNode = * (treeNode + (treeNode->rightChild));
          }
        }
        std::cout << treeNode->returnClass << " ";
        resultMatrix->push_back(treeNode->returnClass);
    	}

    	std::cout << std::endl;
    	std::cout << "Matrix size is: " << resultMatrix->size() << std::endl;
    	pdb::Handle<FFMatrixBlock> resultMatrixBlock = pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex, inNumRow, 1, resultMatrix);
    	return resultMatrixBlock;
    });
  }
};

}

#endif