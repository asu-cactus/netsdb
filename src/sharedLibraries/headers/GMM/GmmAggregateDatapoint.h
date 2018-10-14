#ifndef GMM_AGGREGATE_DATAPOINT_H
#define GMM_AGGREGATE_DATAPOINT_H

#include "DoubleVector.h"
#include "Object.h"
#include "PDBVector.h"

// By Tania, October 2017

using namespace pdb;

// GmmAggregateDatapoint contains a processed datapoint and the responsabilities
// or posterior probabilities (rvalues) of this datapoint for each model
// component
class GmmAggregateDatapoint : public Object {

private:
  DoubleVector datapoint; // Datapoint to be processed
  Vector<double> rvalues; // Responsabilities
  double logLikelihood;

public:
  ENABLE_DEEP_COPY

  GmmAggregateDatapoint() {}

  GmmAggregateDatapoint(DoubleVector datapoint, Vector<double> rvalues,
                        double logLikelihood) {
    this->datapoint = datapoint;
    this->rvalues = rvalues;
    this->logLikelihood = logLikelihood;
  }

  DoubleVector &getDatapoint() { return this->datapoint; }

  Vector<double> &getRvalues() { return this->rvalues; }

  double getLogLikelihood() { return this->logLikelihood; }

  void setLogLikelihood(double logLikelihood) {
    this->logLikelihood = logLikelihood;
  }

  ~GmmAggregateDatapoint() {}
};

#endif
