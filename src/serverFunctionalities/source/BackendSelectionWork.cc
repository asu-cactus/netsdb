#ifndef BACKENDSELECTIONWORK_CC
#define BACKENDSELECTIONWORK_CC

#include "BackendSelectionWork.h"
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

void BackendSelectionWork::execute(PDBBuzzerPtr callerBuzzer) {

    size_t tempPageSize = 1024 * 1 * 1024;

    char logName[100];
    sprintf(logName, "thread%d.log", iter->getId());
    pdb::PDBLoggerPtr logger = make_shared<pdb::PDBLogger>(logName);
    logger->writeLn("BackendSelectionWork: running...");

    // create a new connection to frontend server
    string errMsg;
    pdb::PDBCommunicatorPtr communicatorToFrontEnd = make_shared<pdb::PDBCommunicator>();
    communicatorToFrontEnd->connectToInternetServer(
        logger, server->getConf()->getPort(), "localhost", errMsg);

    NodeID nodeId = server->getNodeID();
    DataProxyPtr proxy = make_shared<DataProxy>(
        nodeId, communicatorToFrontEnd, this->server->getSharedMem(), logger);

    PDBPagePtr page, destPage;
    PageHandlePtr destPageHandle;
    void* tempPage = 0;

    auto filterProc = myQuery->getFilterProcessor();
    auto projProc = myQuery->getProjectionProcessor();
    while (this->iter->hasNext()) {
        page = this->iter->next();
        // page still can be nullptr, so we MUST check nullptr here.
        if (page != nullptr) {
            // tempPageSize = page->getSize();
            // we never did any query work before in this thread.
            tempPage = calloc(tempPageSize, 1);
            filterProc->initialize();
            filterProc->loadOutputPage(tempPage, tempPageSize);


            // load input page
            std::cout << "processing page with pageId=" << page->getPageID() << std::endl;
            filterProc->loadInputPage(page->getBytes());
            while (filterProc->fillNextOutputPage()) {

                // now tempPage is the input page for projection
                // the tempPage was filled, use it as input for projection
                projProc->loadInputPage(tempPage);
                proxy->addUserPage(destDatabaseId, destTypeId, destSetId, destPage);
                destPageHandle = make_shared<PageHandle>(proxy, destPage);
                projProc->initialize();
                projProc->loadOutputPage(destPage->getBytes(), destPage->getSize());


                while (projProc->fillNextOutputPage()) {
                    destPageHandle->unpin();
                    proxy->addUserPage(destDatabaseId, destTypeId, destSetId, destPage);
                    logger->writeLn("BackendSelectionWork: proxy pinned a new page with pageId=");
                    logger->writeInt(destPage->getPageID());
                    destPageHandle = make_shared<PageHandle>(proxy, destPage);
                    projProc->loadOutputPage(destPage->getBytes(), destPage->getSize());
                }


                projProc->finalize();
                projProc->fillNextOutputPage();
                projProc->clearOutputPage();
                projProc->clearInputPage();
                destPageHandle->unpin();
                destPage = nullptr;

                std::cout << "Done part of projection " << std::endl;
                // finished processing tempPage, load a new one.
                filterProc->clearOutputPage();
                free(tempPage);
                tempPage = calloc(tempPageSize, 1);

                std::cout << "loading new temppage " << std::endl;
                filterProc->loadOutputPage(tempPage, tempPageSize);
                std::cout << "done loading new temppage " << std::endl;
            }


            filterProc->finalize();
            filterProc->fillNextOutputPage();
            proxy->addUserPage(destDatabaseId, destTypeId, destSetId, destPage);
            destPageHandle = make_shared<PageHandle>(proxy, destPage);
            projProc->initialize();
            projProc->loadOutputPage(destPage->getBytes(), destPage->getSize());
            projProc->loadInputPage(tempPage);
            while (projProc->fillNextOutputPage()) {
                destPageHandle->unpin();
                proxy->addUserPage(destDatabaseId, destTypeId, destSetId, destPage);
                logger->writeLn("BackendSelectionWork: proxy pinned a new page with pageId=");
                logger->writeInt(destPage->getPageID());
                destPageHandle = make_shared<PageHandle>(proxy, destPage);
                projProc->loadOutputPage(destPage->getBytes(), destPage->getSize());
            }
            projProc->finalize();
            projProc->fillNextOutputPage();
            projProc->clearOutputPage();
            projProc->clearInputPage();
            destPageHandle->unpin();
            destPage = nullptr;

            filterProc->clearInputPage();
            proxy->unpinUserPage(
                nodeId, page->getDbID(), page->getTypeID(), page->getSetID(), page);
            logger->writeLn(
                "BackendSelectionWork: send out unpinPage for source page with pageID:");
            logger->writeInt(page->getPageID());
            filterProc->clearOutputPage();
            free(tempPage);
        }
    }

    callerBuzzer->buzz(PDBAlarm::WorkAllDone, counter);
    return;
}

#endif
