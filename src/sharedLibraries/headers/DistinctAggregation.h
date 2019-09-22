#pragma once

#include <Object.h>
#include <LambdaCreationFunctions.h>
#include "DistinctUrl.h"
#include "ClusterAggregateComp.h"
#include "Link.h"

namespace pdb {

class DistinctAggregation : public ClusterAggregateComp<DistinctUrl, Link, pdb::String, int> {

public:
  
  ENABLE_DEEP_COPY

  DistinctAggregation() = default;

  // the below constructor is NOT REQUIRED
  DistinctAggregation(const std::string &dbName, const std::string &setName) {
    this->setOutput(dbName, setName);
  }

  // the key type must have == and size_t hash () defined
  Lambda<pdb::String> getKeyProjection(Handle<Link> aggMe) override {
    return makeLambdaFromMember(aggMe, to);
  }

  // the value type must have + defined
  Lambda<int> getValueProjection(Handle<Link> aggMe) override {
    return makeLambda(aggMe, [](Handle<Link>& aggMe) {
      return 0;
    });
  }
};

}
