#ifndef INFERENCE_RESULT_PARTITION_H
#define INFERENCE_RESULT_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "RedditComment.h"
#include "PartitionComp.h"

using namespace pdb;

class LabelCommentsPartition : public PartitionComp<std::string, reddit::Comment> {
public:
  ENABLE_DEEP_COPY

  LabelCommentsPartition() {}

  LabelCommentsPartition(std::string dbname, std::string setname) {
    this->setOutput(dbname, setname);
  }

  Lambda<std::string> getProjection(Handle<reddit::Comment> row) override {
    return makeLambda(row, [this](Handle<reddit::Comment> &row) {
      if(row->label == 1){
        return row->author;
      }else{
        return row->subreddit_id;
      }
    });
  }
};

#endif