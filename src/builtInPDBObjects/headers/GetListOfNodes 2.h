
#ifndef GET_LIST_OF_NODES
#define GET_LIST_OF_NODES

#include "Object.h"

//  PRELOAD %GetListOfNodes%

namespace pdb {

// A request to get the list of current cluster nodes from distribution manager master.
class GetListOfNodes : public Object {

public:
    GetListOfNodes() {}
    ~GetListOfNodes() {}

    ENABLE_DEEP_COPY


private:
};
}

#endif
