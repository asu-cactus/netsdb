
#ifndef SRC_CPP_MAIN_STORAGE_HEADERS_PDBFLUSHCONSUMERWORK_H_
#define SRC_CPP_MAIN_STORAGE_HEADERS_PDBFLUSHCONSUMERWORK_H_

#include "PDBWork.h"
#include "PangeaStorageServer.h"
#include <memory>
using namespace std;
class PDBFlushConsumerWork;
typedef shared_ptr<PDBFlushConsumerWork> PDBFlushConsumerWorkPtr;


//this class flushes pages to disk

class PDBFlushConsumerWork : public pdb::PDBWork {
public:
    PDBFlushConsumerWork(FilePartitionID partitionId, pdb::PangeaStorageServer* server);
    ~PDBFlushConsumerWork(){};
    void execute(PDBBuzzerPtr callerBuzzer) override;
    void stop();

private:
    pdb::PangeaStorageServer* server;
    FilePartitionID partitionId;
    bool isStopped;
};


#endif /* SRC_CPP_MAIN_STORAGE_HEADERS_PDBFLUSHCONSUMERWORK_H_ */
