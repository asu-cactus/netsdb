#ifndef FF_AGG_MATRIX_TO_ONE_MATRIX_H
#define FF_AGG_MATRIX_TO_ONE_MATRIX_H

#include "ClusterAggregateComp.h"
#include "FFMatrixBlock.h"
# include "MatrixBlock.h"
#include "FFSingleMatrix.h"
#include "LambdaCreationFunctions.h"

using namespace pdb;

// This aggregation will merge all FFMatrixBlock into a single bigger Matrix,
// which is called FFSingleMatrix
class FFAggMatrixToOneMatrix : public ClusterAggregateComp<FFSingleMatrix, FFMatrixBlock,
                                                int, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFAggMatrixToOneMatrix() {}

  // the key type must have == and size_t hash () defined
  Lambda<int> getKeyProjection(Handle<FFMatrixBlock> aggMe) override {
      return makeLambda(aggMe, [](Handle<FFMatrixBlock>& aggMe) {
        // all FFMatrixBlock will be grouped together
        std::cout << "aggMe->getBlockRowIndex()=" << aggMe->getBlockRowIndex() 
	<< " aggMe->getBlockColIndex()=" << aggMe->getBlockRowIndex() 
	<< " aggMe->getRowNums()=" << aggMe->getRowNums()
        << " aggMe->getColNums()=" << aggMe->getRowNums() << std::endl;
	     	      return 1;
      });
  }

  // the value type must have + defined
    Lambda<FFMatrixBlock> getValueProjection(Handle<FFMatrixBlock> aggMe) override {
        // return the FFmatrixBlock, the aggregation will invoke the overloaded + operator 
        // which is defined in FFMatrixBlock
        return makeLambda(aggMe, [](Handle<FFMatrixBlock>& aggMe) { return *aggMe; });
    }
};

#endif
