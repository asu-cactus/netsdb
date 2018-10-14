#ifndef OBJECTQUERYMODEL_PARTITIONPOLICYFACTORY_CC
#define OBJECTQUERYMODEL_PARTITIONPOLICYFACTORY_CC

#include "PartitionPolicyFactory.h"

namespace pdb {

PartitionPolicyPtr PartitionPolicyFactory::buildPartitionPolicy(PartitionPolicy::Policy policy) {
    switch (policy) {
        case PartitionPolicy::Policy::RANDOM:
            return std::make_shared<RandomPolicy>();
        case PartitionPolicy::Policy::ROUNDROBIN:
            return std::make_shared<RoundRobinPolicy>();
        case PartitionPolicy::Policy::FAIR:
            // TODO: Implement this class
            return nullptr;
        case PartitionPolicy::Policy::DEFAULT:
            // Random policy is the default policy
            return buildDefaultPartitionPolicy();
    }
}

PartitionPolicyPtr PartitionPolicyFactory::buildDefaultPartitionPolicy() {
#ifdef RANDOM_DISPATCHER
    return std::make_shared<RandomPolicy>();
#else
    return std::make_shared<RoundRobinPolicy>();
#endif
}
}

#endif
