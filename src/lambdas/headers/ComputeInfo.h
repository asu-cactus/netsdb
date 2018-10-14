
#ifndef COMPUTE_INFO_H
#define COMPUTE_INFO_H

#include <memory>

namespace pdb {

// this is the base class for parameters that are sent into a pipeline when it is built
class ComputeInfo {

public:
    virtual ~ComputeInfo() {}
};

typedef std::shared_ptr<ComputeInfo> ComputeInfoPtr;
}

#endif
