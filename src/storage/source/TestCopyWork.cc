#ifndef TESTCOPYWORK_CC
#define TESTCOPYWORK_CC

#include "TestCopyWork.h"
#include "PDBPage.h"
#include "Handle.h"
#include "Object.h"
#include "DataProxy.h"
#include "PageHandle.h"
#include "InterfaceFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "PDBVector.h"
#include <string>
#include "SharedEmployee.h"

using namespace std;

TestCopyWork::TestCopyWork(PageCircularBufferIteratorPtr iter,
                           DatabaseID destDatabaseId,
                           UserTypeID destTypeId,
                           SetID destSetId,
                           pdb::HermesExecutionServer* server,
                           atomic_int& counter)
    : counter(counter) {
    this->iter = iter;
    this->destDatabaseId = destDatabaseId;
    this->destTypeId = destTypeId;
    this->destSetId = destSetId;
    this->server = server;
}

// do the actual work

void TestCopyWork::execute(PDBBuzzerPtr callerBuzzer) {
    char logName[100];
    sprintf(logName, "thread%d.log", iter->getId());
    pdb::PDBLoggerPtr logger = make_shared<pdb::PDBLogger>(logName);
    logger->writeLn("TestCopyWork: running...");

    // create a new connection to frontend server
    string errMsg;
    pdb::PDBCommunicatorPtr communicatorToFrontEnd = make_shared<pdb::PDBCommunicator>();
    communicatorToFrontEnd->connectToInternetServer(
        logger, server->getConf()->getPort(), "localhost", errMsg);

    NodeID nodeId = server->getNodeID();
    DataProxyPtr proxy = make_shared<DataProxy>(
        nodeId, communicatorToFrontEnd, this->server->getSharedMem(), logger);

    PDBPagePtr page, destPage;
    proxy->addUserPage(destDatabaseId, destTypeId, destSetId, destPage);
    PageHandlePtr destPageHandle = make_shared<PageHandle>(proxy, destPage);

    // load output page
    pdb::UseTemporaryAllocationBlockPtr blockPtr;
    blockPtr = std::make_shared<pdb::UseTemporaryAllocationBlock>(
        destPageHandle->getWritableBytes(), destPageHandle->getWritableSize());
    pdb::Handle<pdb::Vector<pdb::Handle<SharedEmployee>>> outputVec =
        pdb::makeObject<pdb::Vector<pdb::Handle<SharedEmployee>>>(300000);
    pdb::Handle<pdb::Vector<pdb::Handle<SharedEmployee>>> inputVec;
    while (this->iter->hasNext()) {
        page = this->iter->next();
        if (page != nullptr) {

            // load input page
            std::cout << "processing page with pageId=" << page->getPageID() << std::endl;
            pdb::Record<pdb::Vector<pdb::Handle<SharedEmployee>>>* temp =
                (pdb::Record<pdb::Vector<pdb::Handle<SharedEmployee>>>*)page->getBytes();
            inputVec = temp->getRootObject();
            std::cout << "there are " << inputVec->size() << " objects on the page." << std::endl;


            for (int i = 0; i < inputVec->size(); i++) {
                pdb::Handle<SharedEmployee> object = (*inputVec)[i];
                if (i % 10000 == 0) {
                    std::cout << i << ":";
                    object->print();
                    std::cout << std::endl;
                }
                try {
                    outputVec->push_back(object);
                } catch (pdb::NotEnoughSpace& n) {
                    std::cout << "########################################\n";
                    std::cout << "a page is fully written with " << outputVec->size()
                              << " objects\n";
                    getRecord(outputVec);
                    destPageHandle->unpin();
                    proxy->addUserPage(
                        this->destDatabaseId, this->destTypeId, this->destSetId, destPage);
                    logger->writeLn("TestCopyWork: proxy pinned a new temp page with pageId=");
                    logger->writeInt(destPage->getPageID());
                    destPageHandle = make_shared<PageHandle>(proxy, destPage);

                    blockPtr = nullptr;
                    blockPtr = std::make_shared<pdb::UseTemporaryAllocationBlock>(
                        destPageHandle->getWritableBytes(), destPageHandle->getWritableSize());
                    outputVec = pdb::makeObject<pdb::Vector<pdb::Handle<SharedEmployee>>>(300000);
                    outputVec->push_back(object);
                }
            }
            // clean the page;
            if (proxy->unpinUserPage(
                    nodeId, page->getDbID(), page->getTypeID(), page->getSetID(), page) == false) {
                logger->writeLn("TestCopyWork: can not unpin finished page.");
                destPageHandle->unpin();
                callerBuzzer->buzz(PDBAlarm::QueryError);
                return;
            }
            logger->writeLn("TestCopyWork: send out unpinPage for source page with pageID:");
            logger->writeInt(page->getPageID());
        }
    }
    std::cout << "########################################\n";
    std::cout << "a page is fully written with " << outputVec->size() << " objects\n";
    getRecord(outputVec);
    destPageHandle->unpin();
    callerBuzzer->buzz(PDBAlarm::WorkAllDone, counter);
    return;
}

#endif
