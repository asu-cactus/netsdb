#ifndef AUTHOR_COMMENTS_PARTITION_H
#define AUTHOR_COMMENTS_PARTITION_H

#include "Lambda.h"
#include "LambdaCreationFunctions.h"

#include "RedditComment.h"
#include "PartitionComp.h"

using namespace pdb;

class AuthorCommentsPartition : public PartitionComp<pdb::String, reddit::Comment> {
public:
  ENABLE_DEEP_COPY

  AuthorCommentsPartition() {}

  AuthorCommentsPartition(std::string dbname, std::string setname) {
    this->setOutput(dbname, setname);
  }

  Lambda<pdb::String> getProjection(Handle<reddit::Comment> row) override {
    return makeLambda(row, [this](Handle<reddit::Comment> &row) {
      return row->author;
    });
  }
};

#endif