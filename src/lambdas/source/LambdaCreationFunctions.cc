


#include <cstring>
#include "LambdaCreationFunctions.h"

namespace pdb {

void* someRandomPointer;


int mapToPartitionId (size_t hashVal, int numPartitions) {

#ifndef NO_MOD_PARTITION
    return hashVal % numPartitions;
#else
    return (hashVal / numPartitions) % numPartitions;
#endif

}

}
