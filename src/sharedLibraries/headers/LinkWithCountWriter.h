#pragma once

#include "WriteUserSet.h"
#include "OutgoingURLsCount.h"

using namespace pdb;
class LinkWithCountWriter : public WriteUserSet<OutgoingURLsCount> {

 public:

  ENABLE_DEEP_COPY

  LinkWithCountWriter() = default;

  LinkWithCountWriter(const std::string &dbName, const std::string &setName) {
    this->setOutput(dbName, setName);
  }
};
