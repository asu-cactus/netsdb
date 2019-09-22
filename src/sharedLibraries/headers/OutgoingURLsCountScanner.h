#pragma once

#include "ScanUserSet.h"
#include "OutgoingURLsCount.h"

using namespace pdb;
class OutgoingURLsCountScanner : public ScanUserSet<OutgoingURLsCount> {
 public:

  ENABLE_DEEP_COPY

  OutgoingURLsCountScanner() = default;

  OutgoingURLsCountScanner(const std::string &dbName, const std::string &setName) {
    setDatabaseName(dbName);
    setSetName(setName);
  }
};