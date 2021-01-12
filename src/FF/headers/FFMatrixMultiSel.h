#ifndef FF_MATRIX_MULTISEL_H
#define FF_MATRIX_MULTISEL_H

#include <cassert>
#include <cmath>

#include "MultiSelectionComp.h"

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "PDBString.h"
#include "PDBVector.h"

#include "FFMatrixBlock.h"
#include "InferenceResult.h"

using namespace pdb;

class FFMatrixMultiSel
    : public MultiSelectionComp<InferenceResult, FFMatrixBlock> {

public:
  ENABLE_DEEP_COPY

  FFMatrixMultiSel() {}

  Lambda<bool> getSelection(Handle<FFMatrixBlock> checkMe) override {
    return makeLambda(
        checkMe, [](Handle<FFMatrixBlock> &checkMe) { return true; });
  }

  Lambda<Vector<Handle<InferenceResult>>>
  getProjection(Handle<FFMatrixBlock> checkMe) override {
    return makeLambda(checkMe, [this](Handle<FFMatrixBlock> &checkMe) {
      Vector<Handle<InferenceResult>> result;

      uint32_t I = checkMe->getRowNums();
      // This is the inference output with the probabilities of each label. We have only 2 labels.
      uint32_t J = checkMe->getColNums();

      // assert(J == 2);

      double *checkMeData = checkMe->getValue().rawData->c_ptr();

      for (int i = 0; i < I; i++) {
          int index = checkMe->getBlockRowIndex() * I + i;
          Handle<InferenceResult> inference = makeObject<InferenceResult>(index, checkMe->getBlockRowIndex());
          Vector<double> &inference_results = inference->getInference();
          for (int j = 0; j < 2; j++) {
              inference_results.push_back(checkMeData[i * J + j]);
          }
          inference->dump();
          result.push_back(inference);
      }

      return result;
    });
  }
};


#endif
