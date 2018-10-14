
#ifndef GMM_SAMPLE_SELECTION_H
#define GMM_SAMPLE_SELECTION_H

#include "DoubleVector.h"
#include "Lambda.h"
#include "LambdaCreationFunctions.h"
#include "SelectionComp.h"
#include <cstdlib>
#include <ctime>

using namespace pdb;

// GmmSampleSelection will randomly select several data points from the input
// that will be used to initialize the GMM model.
class GmmSampleSelection : public SelectionComp<DoubleVector, DoubleVector> {

private:
  double fraction;

public:
  ENABLE_DEEP_COPY

  GmmSampleSelection() {}

  GmmSampleSelection(double inputFraction) { this->fraction = inputFraction; }

  // srand has already been invoked in server
  Lambda<bool> getSelection(Handle<DoubleVector> checkMe) override {
    return makeLambda(checkMe, [&](Handle<DoubleVector> &checkMe) {

      double myVal = (double)rand() / (double)RAND_MAX;
      bool ifSample = (myVal <= (this->fraction));
      if (ifSample)
        return true;
      else
        return false;
    });
  }

  Lambda<Handle<DoubleVector>>
  getProjection(Handle<DoubleVector> checkMe) override {
    return makeLambda(checkMe, [](Handle<DoubleVector> &checkMe) {
      std::cout << "I am selected!!";
      checkMe->print();
      return checkMe;
    });
  }
};

#endif
