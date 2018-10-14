
#ifndef PDBFLUSHPRODUCERWORK_H
#define PDBFLUSHPRODUCERWORK_H

#include "PDBWork.h"
#include "PangeaStorageServer.h"
#include <memory>
using namespace std;
class PDBFlushProducerWork;
typedef shared_ptr<PDBFlushProducerWork> PDBFlushProducerWorkPtr;

//this class put pages to the flushing queue

class PDBFlushProducerWork : public pdb::PDBWork {
public:
    PDBFlushProducerWork(pdb::PangeaStorageServer* server);
    ~PDBFlushProducerWork(){};
    void execute(PDBBuzzerPtr callerBuzzer) override;

private:
    pdb::PangeaStorageServer* server;
};


#endif /* PDBFLUSHPRODUCERWORK_H */
