#ifndef INFERENCE_RESULT_PARTITION_H
#define INFERENCE_RESULT_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "RedditComment.h"
#include "PartitionComp.h"

using namespace pdb;

class LabelCommentsPartition : public PartitionComp<int, reddit::Comment> {
public:
  ENABLE_DEEP_COPY

  LabelCommentsPartition() {}

  LabelCommentsPartition(std::string dbname, std::string setname) {
    this->setOutput(dbname, setname);
  }

  Lambda<int> getProjection(Handle<reddit::Comment> checkMe) override {
    return makeLambda(checkMe, [this](Handle<reddit::Comment> &checkMe) {
      return checkMe->label;
    });
  }
};

#endif