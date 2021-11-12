#ifndef EMBEDDING_LOOKUP_SPARSE_H
#define EMBEDDING_LOOKUP_SPARSE_H

#include "FFMatrixBlock.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "MultiSelectionComp.h"
#include "PDBVector.h"
#include "EmbeddingSegment.h"
#include <eigen3/Eigen/Dense>

using namespace pdb;

class EmbeddingLookupSparse : public MultiSelectionComp<EmbeddingSegment, FFMatrixBlock> {
  private:
    Handle<Vector<int>> id_vector = nullptr;
    int dimension_size = 500;
  public:
    ENABLE_DEEP_COPY

    EmbeddingLookupSparse() {

    }

    EmbeddingLookupSparse(Handle<Vector<int>> id_vector, int dimension_size) {
        this->id_vector = makeObject<Vector<int>>();
    	this->id_vector = id_vector;
	this->dimension_size = dimension_size;
    }

    Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override {
        return makeLambda(checkMe,
                          [this](Handle<FFMatrixBlock> &checkMe) { 
			       // load the metadata
                               uint32_t inNumRow = checkMe->getRowNums();
                               uint32_t inBlockRowIndex = checkMe->getBlockRowIndex();
			       int minRow = inBlockRowIndex * inNumRow;
			       int maxRow = minRow + inNumRow - 1;

			       for (int i = 0; i < this->id_vector->size(); i++) {
			           int id = (*(this->id_vector))[i];
				   if ((id >= minRow) && (id <= maxRow)) {
				       return true;
				   } 
			       }
			       return false;
			  });
    }

    Lambda<Vector<Handle<EmbeddingSegment>>>
    getProjection(Handle<FFMatrixBlock> in) override {
        return makeLambda(in, [this](Handle<FFMatrixBlock> &in) {

	    // create a container for output embeddings
	    Vector<Handle<EmbeddingSegment>> ret; 
            // load the metadata
            uint32_t inNumRow = in->getRowNums();
            uint32_t inNumCol = in->getColNums();
            uint32_t inBlockRowIndex = in->getBlockRowIndex();
            uint32_t inBlockColIndex = in->getBlockColIndex();
            int minRow = inBlockRowIndex * inNumRow;
            int maxRow = minRow + inNumRow - 1;
	    int minCol = inBlockColIndex * inNumCol;
	    for (int i = 0; i < this->id_vector->size(); i++) {
	        int id =  (*(this->id_vector))[i];
                if ((id >= minRow) && (id <= maxRow)) {
		    Handle<Vector<double>> embedding = makeObject<Vector<double>>(this->dimension_size, this->dimension_size);
                    double* dest = embedding->c_ptr()+minCol;
		    Handle<Vector<double>> rawData = in->data.rawData;
		    double* src = rawData->c_ptr();
		    memcpy(src, dest, inNumCol*sizeof(double));
		     
                    Handle<EmbeddingSegment> segment = makeObject<EmbeddingSegment>(id, inBlockColIndex, inNumCol, embedding );
		    ret.push_back(segment);
                }
            }
            return ret;
        });
    }
};

#endif
