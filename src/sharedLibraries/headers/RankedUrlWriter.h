#pragma once

#include "WriteUserSet.h"
#include "RankedUrl.h"

using namespace pdb;
class RankedUrlWriter : public WriteUserSet<RankedUrl> {

 public:

  ENABLE_DEEP_COPY

  RankedUrlWriter() = default;

  RankedUrlWriter(const std::string &dbName, const std::string &setName) {
    this->setOutput(dbName, setName);
  }
};
