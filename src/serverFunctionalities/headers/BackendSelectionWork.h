
#ifndef BACKENDSELECTIONWORK_H
#define BACKENDSELECTIONWORK_H

#include "PDBBuzzer.h"
#include "PageCircularBufferIterator.h"
#include "DataTypes.h"
#include "HermesExecutionServer.h"
#include "Selection.h"
#include <memory>

using namespace std;

class BackendSelectionWork;
typedef shared_ptr<BackendSelectionWork> SelectionWorkPtr;

class BackendSelectionWork : public pdb::PDBWork {
public:
    BackendSelectionWork(PageCircularBufferIteratorPtr iterIn,
                         DatabaseID destDatabaseIdIn,
                         UserTypeID destTypeIdIn,
                         SetID destSetIdIn,
                         pdb::HermesExecutionServer* serverIn,
                         int& counterIn,
                         pdb::Handle<pdb::Selection<pdb::Object, pdb::Object>> myQueryIn)
        : iter(iterIn),
          destDatabaseId(destDatabaseIdIn),
          destTypeId(destTypeIdIn),
          destSetId(destSetIdIn),
          server(serverIn),
          counter(counterIn),
          myQuery(myQueryIn) {}

    // do the actual work
    void execute(PDBBuzzerPtr callerBuzzer) override;

private:
    PageCircularBufferIteratorPtr iter;
    DatabaseID destDatabaseId;
    UserTypeID destTypeId;
    SetID destSetId;
    pdb::HermesExecutionServer* server;
    int& counter;
    pdb::Handle<pdb::Selection<pdb::Object, pdb::Object>> myQuery;
};


#endif /* TESTCOPYWORK_H */
