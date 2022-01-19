#ifndef GETSETWORK_H
#define GETSETWORK_H

#include "PDBBuzzer.h"
#include "PageCircularBufferIterator.h"
#include "HermesExecutionServer.h"
#include "TreeNode.h"
#include <memory>
#include <atomic>

using namespace std;
class GetSetWork;
typedef shared_ptr<GetSetWork> GetSetWorkPtr;

/**
 * This class illustrates how a backend server can communicate with frontend server
 * to get user data stored as a set in storage.
 */

class GetSetWork : public pdb::PDBWork {
public:
    GetSetWork(PageCircularBufferIteratorPtr iter,
                 pdb::HermesExecutionServer* server,
                 atomic_int& counter);

    void execute(PDBBuzzerPtr callerBuzzer) override;

    // do the actual work
    std::vector<decisiontree::Node> runActualWork(PDBBuzzerPtr callerBuzzer);
    
private:
    PageCircularBufferIteratorPtr iter;
    pdb::HermesExecutionServer* server;
    atomic_int& counter;
};

#endif /* GETSETWORK_H */