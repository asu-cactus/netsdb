
#ifndef TESTCOPYWORK_H
#define TESTCOPYWORK_H

#include "PDBBuzzer.h"
#include "PageCircularBufferIterator.h"
#include "DataTypes.h"
#include "HermesExecutionServer.h"
#include <memory>

using namespace std;

class TestCopyWork;
typedef shared_ptr<TestCopyWork> TestCopyWorkPtr;

/**
 * This class illustrates how a backend server can communicate with frontend server
 * to scan data and copy data to another set in storage.
 */

class TestCopyWork : public pdb::PDBWork {
public:
    TestCopyWork(PageCircularBufferIteratorPtr iter,
                 DatabaseID destDatabaseId,
                 UserTypeID destTypeId,
                 SetID destSetId,
                 pdb::HermesExecutionServer* server,
                 atomic_int& counter);

    // do the actual work
    void execute(PDBBuzzerPtr callerBuzzer) override;

private:
    PageCircularBufferIteratorPtr iter;
    pdb::HermesExecutionServer* server;
    DatabaseID destDatabaseId;
    UserTypeID destTypeId;
    SetID destSetId;
    atomic_int& counter;
};


#endif /* TESTCOPYWORK_H */
