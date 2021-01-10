
#ifndef HASH_PARTITION_WORK_H
#define HASH_PARTITION_WORK_H

#include "PDBBuzzer.h"
#include "PageCircularBufferIterator.h"
#include "PDBCommunicator.h"
#include "Configuration.h"
#include "Handle.h"
#include "TupleSetJobStage.h"
#include "PipelineStage.h"
#include <memory>

using namespace std;
class HashPartitionWork;
typedef shared_ptr<HashPartitionWork> HashPartitionWorkPtr;

/**
 * This class illustrates how a BackEnd server stores received partitions in local or shuffles
 *  received partitions to remote nodes.
 */

namespace pdb {

class HashPartitionWork : public pdb::PDBWork {
public:
    HashPartitionWork(int id, PageCircularBufferIteratorPtr iter, PipelineStage * stage, 
         atomic_int& counter);
    ~HashPartitionWork();

    // do the actual work
    void execute(PDBBuzzerPtr callerBuzzer) override;

private:
    int id;
    PageCircularBufferIteratorPtr iter;
    PipelineStage * stage;
    Handle<TupleSetJobStage> jobStage;
    ConfigurationPtr conf;
    atomic_int& counter;
    pthread_mutex_t connection_mutex;
    PDBLoggerPtr logger;
};

}

#endif 
