
#ifndef SRC_CPP_MAIN_STORAGE_HEADERS_PDBSCANWORK_H_
#define SRC_CPP_MAIN_STORAGE_HEADERS_PDBSCANWORK_H_

#include "PDBBuzzer.h"
#include "PageCircularBufferIterator.h"
#include "PDBCommunicator.h"
#include "PangeaStorageServer.h"
#include <memory>
using namespace std;
class PDBScanWork;
typedef shared_ptr<PDBScanWork> PDBScanWorkPtr;

/**
 * This class illustrates how a FrontEnd server scan set data and communicate with BackEnd server
 * to notify loaded pages.
 */

class PDBScanWork : public pdb::PDBWork {
public:
    PDBScanWork(PageIteratorPtr iter, pdb::PangeaStorageServer* storage, int& counter);
    ~PDBScanWork();
    bool sendPagePinned(pdb::PDBCommunicatorPtr myCommunicator,
                        bool morePagesToPin,
                        NodeID nodeId,
                        DatabaseID dbId,
                        UserTypeID typeId,
                        SetID setId,
                        PageID pageId,
                        size_t pageSize,
                        size_t offset);

    bool acceptPagePinnedAck(pdb::PDBCommunicatorPtr myCommunicator,
                             bool& wasError,
                             string& info,
                             string& errMsg);

    // do the actual work
    void execute(PDBBuzzerPtr callerBuzzer) override;

private:
    PageIteratorPtr iter;
    pdb::PangeaStorageServer* storage;
    int& counter;
    pthread_mutex_t connection_mutex;
};


#endif /* SRC_CPP_MAIN_STORAGE_HEADERS_PDBSCANWORK_H_ */
