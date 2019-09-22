#pragma once

#include "ScanUserSet.h"
#include "Link.h"

using namespace pdb;
class LinkScanner : public ScanUserSet<Link> {
 public:

  ENABLE_DEEP_COPY

  LinkScanner() = default;

  LinkScanner(const std::string &dbName, const std::string &setName) {
    setDatabaseName(dbName);
    setSetName(setName);
  }
};