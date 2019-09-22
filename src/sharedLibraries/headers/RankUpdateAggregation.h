#pragma once

#include <Object.h>
#include <LambdaCreationFunctions.h>
#include "DistinctUrl.h"
#include "ClusterAggregateComp.h"
#include "Link.h"
#include "RankedUrl.h"
#include "LinkWithValue.h"

namespace pdb {

class RankUpdateAggregation : public ClusterAggregateComp<RankedUrl, LinkWithValue, pdb::String, float> {

 public:

  ENABLE_DEEP_COPY

  RankUpdateAggregation() = default;

  // the below constructor is NOT REQUIRED
  RankUpdateAggregation(const std::string &dbName, const std::string &setName) {
    this->setOutput(dbName, setName);
  }

  // the key type must have == and size_t hash () defined
  Lambda<pdb::String> getKeyProjection(Handle<LinkWithValue> aggMe) override {
    return makeLambdaFromMember(aggMe, to);
  }

  // the value type must have + defined
  Lambda<float> getValueProjection(Handle<LinkWithValue> aggMe) override {
    return makeLambda(aggMe, [](Handle<LinkWithValue>& aggMe) {
      return aggMe->value;
    });
  }
};

}
