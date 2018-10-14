
#ifndef OBJECTQUERYMODEL_PARTITIONPOLICYFACTORY_H
#define OBJECTQUERYMODEL_PARTITIONPOLICYFACTORY_H

#include "PartitionPolicy.h"
#include "RandomPolicy.h"
#include "RoundRobinPolicy.h"

namespace pdb {

class PartitionPolicyFactory {

public:
    static PartitionPolicyPtr buildPartitionPolicy(PartitionPolicy::Policy policy);

    static PartitionPolicyPtr buildDefaultPartitionPolicy();
};
}
#endif  // OBJECTQUERYMODEL_PARTITIONPOLICYFACTORY_H
