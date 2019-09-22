#pragma once

#include <Object.h>
#include <LambdaCreationFunctions.h>
#include "DistinctUrl.h"
#include "ClusterAggregateComp.h"
#include "Link.h"
#include "OutgoingURLsCount.h"

namespace pdb {

class LinkWithCountAggregation : public ClusterAggregateComp<OutgoingURLsCount, Link, String, uint64_t> {

 public:

  ENABLE_DEEP_COPY

  LinkWithCountAggregation() = default;

  // the below constructor is NOT REQUIRED
  LinkWithCountAggregation(const std::string &dbName, const std::string &setName) {
    this->setOutput(dbName, setName);
  }

  // the key type must have == and size_t hash () defined
  Lambda<String> getKeyProjection(Handle<Link> aggMe) override {
    return makeLambdaFromMember(aggMe, from);
  }

  // the value type must have + defined
  Lambda<uint64_t> getValueProjection(Handle<Link> aggMe) override {
    return makeLambda(aggMe, [](Handle<Link>& aggMe) {
      return (uint64_t) 1;
    });
  }
};

}
