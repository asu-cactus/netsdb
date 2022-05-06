#ifndef BASELINE_JOIN_DT_N_H
#define BASELINE_JOIN_DT_N_H

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
	
class JoinforBaselineDT_N : public JoinComp<FFMatrixBlock, Tree, FFMatrixBlock> {

 public:

  ENABLE_DEEP_COPY

  JoinforBaselineDT_N() = default;

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
      
      // inference
      pdb::Handle<pdb::Vector<double>> resultMatrix = in1->predict(in2);
    	pdb::Handle<FFMatrixBlock> resultMatrixBlock = pdb::makeObject<FFMatrixBlock>(inBlockRowIndex, inBlockColIndex, inNumRow, 1, resultMatrix);
    	return resultMatrixBlock;
    });
  }
};

}

#endif