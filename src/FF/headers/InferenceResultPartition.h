#ifndef INFERENCE_RESULT_PARTITION_H
#define INFERENCE_RESULT_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "InferenceResult.h"
#include "PartitionComp.h"

using namespace pdb;

class InferenceResultPartition : public PartitionComp<int, InferenceResult> {
public:
  ENABLE_DEEP_COPY

  InferenceResultPartition() {}

  InferenceResultPartition(std::string dbname, std::string setname) {
    this->setOutput(dbname, setname);
  }

  Lambda<int> getProjection(Handle<InferenceResult> checkMe) override {
    return makeLambda(checkMe, [this](Handle<InferenceResult> &checkMe) {
      return checkMe->getKey();
    });
  }
};

#endif
