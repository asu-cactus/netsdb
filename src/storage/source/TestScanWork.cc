#ifndef TESTSCANWORK_CC
#define TESTSCANWORK_CC

#include "TestScanWork.h"
#include "PDBVector.h"
#include "InterfaceFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "PDBPage.h"
#include "DataProxy.h"
#include <string>
using namespace std;

TestScanWork::TestScanWork(PageCircularBufferIteratorPtr iter,
                           pdb::HermesExecutionServer* server,
                           atomic_int& counter)
    : counter(counter) {
    this->iter = iter;
    this->server = server;
}

// do the actual work

void TestScanWork::execute(PDBBuzzerPtr callerBuzzer) {
    pdb::PDBLoggerPtr logger = server->getLogger();
    logger->writeLn("TestScanWork: running...");

    // create a new connection to frontend server
    string errMsg;
    pdb::PDBCommunicatorPtr communicatorToFrontEnd = make_shared<pdb::PDBCommunicator>();
    communicatorToFrontEnd->connectToInternetServer(
        logger, server->getConf()->getPort(), "localhost", errMsg);

    NodeID nodeId = server->getNodeID();
    DataProxyPtr proxy = make_shared<DataProxy>(
        nodeId, communicatorToFrontEnd, this->server->getSharedMem(), logger);

    PDBPagePtr page;
    while (this->iter->hasNext()) {
        page = this->iter->next();
        // page still can be nullptr, so we MUST check nullptr here.
        if (page != nullptr) {
            std::cout << "processing page with pageId=" << page->getPageID() << std::endl;
            pdb::Record<pdb::Vector<pdb::Handle<pdb::Object>>>* temp =
                (pdb::Record<pdb::Vector<pdb::Handle<pdb::Object>>>*)page->getBytes();
            pdb::Handle<pdb::Vector<pdb::Handle<pdb::Object>>> objects = temp->getRootObject();
            std::cout << "there are " << objects->size() << " objects on the page." << std::endl;

            // clean the page;
            if (proxy->unpinUserPage(
                    nodeId, page->getDbID(), page->getTypeID(), page->getSetID(), page) == false) {
                logger->writeLn("TestScanWork: can not add finished page to cleaner.");
                callerBuzzer->buzz(PDBAlarm::QueryError);
                return;
            }
        }
    }
    callerBuzzer->buzz(PDBAlarm::WorkAllDone, counter);
    return;
}

#endif
