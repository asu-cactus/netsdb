#ifndef GETSETWORK_CC
#define GETSETWORK_CC

#include "GetSetWork.h"
#include "PDBVector.h"
#include "InterfaceFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "PDBPage.h"
#include "DataProxy.h"
#include "TreeNode.h"
#include <string>

using namespace std;

GetSetWork::GetSetWork(PageCircularBufferIteratorPtr iter,
                           pdb::HermesExecutionServer* server,
                           atomic_int& counter)
    : counter(counter) {
    this->iter = iter;
    this->server = server;
}

// do the actual work

std::vector<decisiontree::Node> GetSetWork::runActualWork(PDBBuzzerPtr callerBuzzer) {
    pdb::PDBLoggerPtr logger = server->getLogger();
    logger->writeLn("GetSetWork: running...");

    // create a new connection to frontend server
    string errMsg;
    pdb::PDBCommunicatorPtr communicatorToFrontEnd = make_shared<pdb::PDBCommunicator>();
    communicatorToFrontEnd->connectToInternetServer(
        logger, server->getConf()->getPort(), "localhost", errMsg);

    NodeID nodeId = server->getNodeID();
    DataProxyPtr proxy = make_shared<DataProxy>(
        nodeId, communicatorToFrontEnd, this->server->getSharedMem(), logger);

    PDBPagePtr page;
    std::vector<decisiontree::Node> vect;
    while (this->iter->hasNext()) {
        page = this->iter->next();
        // page still can be nullptr, so we MUST check nullptr here.
        if (page != nullptr) {
            std::cout << "processing page with pageId=" << page->getPageID() << std::endl;
            pdb::Record<pdb::Vector<pdb::Handle<decisiontree::Node>>>* temp =
                (pdb::Record<pdb::Vector<pdb::Handle<decisiontree::Node>>>*)page->getBytes();
            pdb::Handle<pdb::Vector<pdb::Handle<decisiontree::Node>>> inputVec = temp->getRootObject();
            int vecSize = inputVec->size();
            for (int j = 0; j < vecSize; j++) {
                pdb::Handle<decisiontree::Node> thisNodePtr = (*inputVec)[j];
                // the following will build a decisiontree::Node object
                decisiontree::Node thisNode = decisiontree::Node(thisNodePtr->nodeID,thisNodePtr->indexID,thisNodePtr->isLeaf,thisNodePtr->leftChild,thisNodePtr->rightChild,thisNodePtr->returnClass);
                // testing purpose
                std::cout << "NodeID is: " << thisNode.nodeID << std::endl;
                vect.push_back(thisNode);
            }

            // clean the page;
            if (proxy->unpinUserPage(
                    nodeId, page->getDbID(), page->getTypeID(), page->getSetID(), page) == false) {
                logger->writeLn("GetSetWork: can not add finished page to cleaner.");
                callerBuzzer->buzz(PDBAlarm::QueryError);
                return vect;
            }
        }
    }

    callerBuzzer->buzz(PDBAlarm::WorkAllDone, counter);
    return vect;
}

void GetSetWork::execute(PDBBuzzerPtr callerBuzzer) {
    pdb::PDBLoggerPtr logger = server->getLogger();
    logger->writeLn("GetSetWork: running...");

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
                logger->writeLn("GetSetWork: can not add finished page to cleaner.");
                callerBuzzer->buzz(PDBAlarm::QueryError);
                return;
            }
        }
    }
    callerBuzzer->buzz(PDBAlarm::WorkAllDone, counter);
    return;
}

#endif