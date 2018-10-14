
#ifndef TESTSCANWORK_H
#define TESTSCANWORK_H

#include "PDBBuzzer.h"
#include "PageCircularBufferIterator.h"
#include "HermesExecutionServer.h"
#include <memory>
using namespace std;
class TestScanWork;
typedef shared_ptr<TestScanWork> TestScanWorkPtr;

/**
 * This class illustrates how a backend server can communicate with frontend server
 * to scan user data stored as a set in storage.
 */

class TestScanWork : public pdb::PDBWork {
public:
    TestScanWork(PageCircularBufferIteratorPtr iter,
                 pdb::HermesExecutionServer* server,
                 int& counter);

    // do the actual work
    void execute(PDBBuzzerPtr callerBuzzer) override;

private:
    PageCircularBufferIteratorPtr iter;
    pdb::HermesExecutionServer* server;
    int& counter;
};


#endif /* TESTSCANWORK_H */
