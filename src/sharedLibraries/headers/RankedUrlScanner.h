#pragma once

#include "ScanUserSet.h"
#include "RankedUrl.h"

using namespace pdb;
class RankedUrlScanner : public ScanUserSet<RankedUrl> {
 public:

  ENABLE_DEEP_COPY

  RankedUrlScanner() = default;

  RankedUrlScanner(const std::string &dbName, const std::string &setName) {
    setDatabaseName(dbName);
    setSetName(setName);
  }
};