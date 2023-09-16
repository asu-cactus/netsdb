

#ifndef DATA_PROXY_H
#define DATA_PROXY_H

#include "PDBDebug.h"
#include "DataProxy.h"
#include "InterfaceFunctions.h"
#include "UseTemporaryAllocationBlock.h"
#include "Object.h"
#include "Handle.h"
#include "StorageAddTempSet.h"
#include "StorageAddTempSetResult.h"
#include "StoragePinPage.h"
#include "StoragePinBytes.h"
#include "StorageUnpinPage.h"
#include "SimpleRequestResult.h"
#include "StoragePagePinned.h"
#include "StorageBytesPinned.h"
#include "StorageRemoveTempSet.h"
#include "StorageGetStats.h"
#include "CloseConnection.h"
#include "Configuration.h"



#ifndef MAX_RETRIES
#define MAX_RETRIES 5
#endif

DataProxy::DataProxy(NodeID nodeId,
                     pdb::PDBCommunicatorPtr communicator,
                     SharedMemPtr shm,
                     pdb::PDBLoggerPtr logger) {
    this->nodeId = nodeId;
    this->communicator = communicator;
    this->shm = shm;
    this->logger = logger;
    this->communicator->setLongConnection(true);
}

DataProxy::~DataProxy() {}

bool DataProxy::addTempSet(string setName, SetID& setId, bool needMem, int numTries) {
    if (numTries == MAX_RETRIES) {
        return false;
    }
    if (numTries > 0) {
        logger->error(std::string("DataProxy: addTempSet with numTries=") +
                      std::to_string(numTries));
    }
    string errMsg;
    if (this->communicator->isSocketClosed() == true) {
        std::cout << "ERROR in DataProxy: connection is closed" << std::endl;
        logger->error("DataProxy: connection is closed, to reconnect");
        if (communicator->reconnect(errMsg)) {
            std::cout << errMsg << std::endl;
            logger->error(std::string("DataProxy: reconnect failed with errMsg") + errMsg);
            return false;
        }
    }
    if (needMem == true) {
        // create an AddSet object
        {
            const pdb::UseTemporaryAllocationBlock myBlock{1024};
            pdb::Handle<pdb::StorageAddTempSet> msg =
                pdb::makeObject<pdb::StorageAddTempSet>(setName);
            // we don't know the SetID to be added, the frontend will assign one.
            // send the message out
            if (!this->communicator->sendObject<pdb::StorageAddTempSet>(msg, errMsg)) {
                // We reserve Database 0 and Type 0 as temp data
                cout << "Sending object failure: " << errMsg << "\n";
                return addTempSet(setName, setId, needMem, numTries + 1);
            }
        }

        // receive the StorageAddSetResult message
        {
            size_t objectSize = this->communicator->getSizeOfNextObject();
            if (objectSize == 0) {
                cout << "Receiving ack failure" << std::endl;
                return addTempSet(setName, setId, needMem, numTries + 1);
            }
            const pdb::UseTemporaryAllocationBlock myBlock{objectSize};
            bool success;
            pdb::Handle<pdb::StorageAddTempSetResult> ack =
                this->communicator->getNextObject<pdb::StorageAddTempSetResult>(success, errMsg);

            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return addTempSet(setName, setId, needMem, numTries + 1);
            }
            if (success == true) {
                setId = ack->getTempSetID();
            }
            return success;
        }
    } else {

        {
            pdb::Handle<pdb::StorageAddTempSet> msg =
                pdb::makeObject<pdb::StorageAddTempSet>(setName);
            // we don't know the SetID to be added, the frontend will assign one.
            // send the message out
            if (!this->communicator->sendObject<pdb::StorageAddTempSet>(msg, errMsg)) {
                // We reserve Database 0 and Type 0 as temp data
                cout << "Sending object failure: " << errMsg << "\n";
                return addTempSet(setName, setId, needMem, numTries + 1);
            }
        }

        // receive the StorageAddSetResult message
        {
            bool success;
            pdb::Handle<pdb::StorageAddTempSetResult> ack =
                this->communicator->getNextObject<pdb::StorageAddTempSetResult>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return addTempSet(setName, setId, needMem, numTries + 1);
            }
            if (success == true) {
                setId = ack->getTempSetID();
            }
            return success;
        }
    }
}


bool DataProxy::removeTempSet(SetID setId, bool needMem, int numTries) {
    if (numTries == MAX_RETRIES) {
        return false;
    }
    if (numTries > 0) {
        logger->error(std::string("DataProxy: removeTempSet with numTries=") +
                      std::to_string(numTries));
    }
    string errMsg;
    if (this->communicator->isSocketClosed() == true) {
        std::cout << "ERROR in DataProxy: connection is closed" << std::endl;
        logger->error("DataProxy: connection is closed, to reconnect");
        if (communicator->reconnect(errMsg)) {
            std::cout << errMsg << std::endl;
            logger->error(std::string("DataProxy: reconnect failed with errMsg") + errMsg);
            return false;
        }
    }
    // create a RemoveSet object

    if (needMem == true) {
        {
            const pdb::UseTemporaryAllocationBlock myBlock{1024};
            pdb::Handle<pdb::StorageRemoveTempSet> msg =
                pdb::makeObject<pdb::StorageRemoveTempSet>(setId);

            // send the message out
            if (!this->communicator->sendObject<pdb::StorageRemoveTempSet>(msg, errMsg)) {
                cout << "Sending object failure: " << errMsg << "\n";
                return removeTempSet(setId, needMem, numTries + 1);
            }
        }

        // receive the SimpleRequestResult message
        {
            bool success;
            size_t objectSize = this->communicator->getSizeOfNextObject();
            if (objectSize == 0) {
                std::cout << "Receiving ack failure" << std::endl;
                return removeTempSet(setId, needMem, numTries + 1);
            }
            const pdb::UseTemporaryAllocationBlock myBlock{objectSize};
            pdb::Handle<pdb::SimpleRequestResult> ack =
                this->communicator->getNextObject<pdb::SimpleRequestResult>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return removeTempSet(setId, needMem, numTries + 1);
            }

            return success && (ack->getRes().first);
        }
    } else {
        {
            pdb::Handle<pdb::StorageRemoveTempSet> msg =
                pdb::makeObject<pdb::StorageRemoveTempSet>(setId);

            // send the message out
            if (!this->communicator->sendObject<pdb::StorageRemoveTempSet>(msg, errMsg)) {
                cout << "Sending object failure: " << errMsg << "\n";
                return removeTempSet(setId, needMem, numTries + 1);
            }
        }

        // receive the SimpleRequestResult message
        {
            bool success;
            pdb::Handle<pdb::SimpleRequestResult> ack =
                this->communicator->getNextObject<pdb::SimpleRequestResult>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return removeTempSet(setId, needMem, numTries + 1);
            }
            return success && (ack->getRes().first);
        }
    }
}

// page will be pinned at Storage Server
bool DataProxy::addTempPage(SetID setId, PDBPagePtr& page, bool needMem, int numTries) {
    return addUserPage(0, 0, setId, page, needMem, numTries);
}

bool DataProxy::addUserPage(
    DatabaseID dbId, UserTypeID typeId, SetID setId, PDBPagePtr& page, bool needMem, int numTries) {
    if (numTries == MAX_RETRIES) {
        return false;
    }
    if (numTries > 0) {
        logger->error(std::string("DataProxy: addUserPage with numTries=") +
                      std::to_string(numTries));
    }
    string errMsg;
    if (this->communicator->isSocketClosed() == true) {
        std::cout << "ERROR in DataProxy: connection is closed" << std::endl;
        logger->error("DataProxy: connection is closed, to reconnect");
        if (communicator->reconnect(errMsg)) {
            std::cout << errMsg << std::endl;
            logger->error(std::string("DataProxy: reconnect failed with errMsg") + errMsg);
            return false;
        }
    }
    if (needMem == true) {
        // create a PinPage object
        {
            const pdb::UseTemporaryAllocationBlock myBlock{2048};
            pdb::Handle<pdb::StoragePinPage> msg = pdb::makeObject<pdb::StoragePinPage>();
            // We reserve Database 0 and Type 0 as temp data
            msg->setNodeID(this->nodeId);
            msg->setDatabaseID(dbId);
            msg->setUserTypeID(typeId);
            msg->setSetID(setId);
            msg->setWasNewPage(true);
            // send the message out
            if (!this->communicator->sendObject<pdb::StoragePinPage>(msg, errMsg)) {
                cout << "DataProxy.AddUserPage(): Sending object failure: " << errMsg << "\n";
                return addUserPage(dbId, typeId, setId, page, needMem, numTries + 1);
            }
        }

        // receive the PagePinned object
        {
            size_t objectSize = this->communicator->getSizeOfNextObject();
            if (objectSize == 0) {
                std::cout << "Receive ack failure" << std::endl;
                return addUserPage(dbId, typeId, setId, page, needMem, numTries + 1);
            }
            PDB_COUT << "DataProxy: to allocate memory block for PagePinned object" << std::endl;
            const pdb::UseTemporaryAllocationBlock myBlock{objectSize};
            PDB_COUT << "DataProxy: memory block allocated" << std::endl;
            bool success;
            pdb::Handle<pdb::StoragePagePinned> ack =
                this->communicator->getNextObject<pdb::StoragePagePinned>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return addUserPage(dbId, typeId, setId, page, needMem, numTries + 1);
            }
            char* dataIn = (char*)this->shm->getPointer(ack->getSharedMemOffset());
            page = make_shared<PDBPage>(dataIn,
                                        ack->getNodeID(),
                                        ack->getDatabaseID(),
                                        ack->getUserTypeID(),
                                        ack->getSetID(),
                                        ack->getPageID(),
                                        ack->getPageSize(),
                                        ack->getSharedMemOffset());
            page->setPinned(true);
            page->setDirty(true);
            return success;
        }
    } else {

        // create a PinPage object
        {
            pdb::Handle<pdb::StoragePinPage> msg = pdb::makeObject<pdb::StoragePinPage>();
            // We reserve Database 0 and Type 0 as temp data
            msg->setNodeID(this->nodeId);
            msg->setDatabaseID(dbId);
            msg->setUserTypeID(typeId);
            msg->setSetID(setId);
            msg->setWasNewPage(true);
            // send the message out
            if (!this->communicator->sendObject<pdb::StoragePinPage>(msg, errMsg)) {
                cout << "Sending object failure: " << errMsg << "\n";
                return addUserPage(dbId, typeId, setId, page, needMem, numTries + 1);
            }
        }

        // receive the PagePinned object
        {
            bool success;
            pdb::Handle<pdb::StoragePagePinned> ack =
                this->communicator->getNextObject<pdb::StoragePagePinned>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return addUserPage(dbId, typeId, setId, page, needMem, numTries + 1);
            }
            char* dataIn = (char*)this->shm->getPointer(ack->getSharedMemOffset());
            page = make_shared<PDBPage>(dataIn,
                                        ack->getNodeID(),
                                        ack->getDatabaseID(),
                                        ack->getUserTypeID(),
                                        ack->getSetID(),
                                        ack->getPageID(),
                                        ack->getPageSize(),
                                        ack->getSharedMemOffset());
            page->setPinned(true);
            page->setDirty(true);
            return success;
        }
    }
}

bool DataProxy::pinBytes(DatabaseID dbId,
                         UserTypeID typeId,
                         SetID setId,
                         size_t sizeOfBytes,
                         void* bytes,
                         bool needMem,
                         int numTries) {
    if (numTries == MAX_RETRIES) {
        return false;
    }
    if (numTries > 0) {
        logger->error(std::string("DataProxy: pinBytes with numTries=") + std::to_string(numTries));
    }
    string errMsg;
    if (this->communicator->isSocketClosed() == true) {
        std::cout << "ERROR in DataProxy: connection is closed" << std::endl;
        logger->error("DataProxy: connection is closed, to reconnect");
        if (communicator->reconnect(errMsg)) {
            std::cout << errMsg << std::endl;
            logger->error(std::string("DataProxy: reconnect failed with errMsg") + errMsg);
            return false;
        }
    }
    if (needMem == true) {
        // create a PinPage object
        {
            const pdb::UseTemporaryAllocationBlock myBlock{2048};
            pdb::Handle<pdb::StoragePinBytes> msg = pdb::makeObject<pdb::StoragePinBytes>();
            msg->setNodeID(this->nodeId);
            msg->setDatabaseID(dbId);
            msg->setUserTypeID(typeId);
            msg->setSetID(setId);
            msg->setSizeOfBytes(sizeOfBytes);
            // send the message out
            if (!this->communicator->sendObject<pdb::StoragePinBytes>(msg, errMsg)) {
                cout << "DataProxy.AddUserPage(): Sending object failure: " << errMsg << "\n";
                sleep(numTries + 1);
                return pinBytes(dbId, typeId, setId, sizeOfBytes, bytes, needMem, numTries + 1);
            }
            std::cout << "Sent pdb::StoragePinBytes message" << std::endl;
        }

        // receive the PagePinned object
        {
            size_t objectSize = this->communicator->getSizeOfNextObject();
            if (objectSize == 0) {
                std::cout << "Receive ack failure" << std::endl;
                sleep(numTries + 1);
                return pinBytes(dbId, typeId, setId, sizeOfBytes, bytes, needMem, numTries + 1);
            }
            PDB_COUT << "DataProxy: to allocate memory block for BytesPinned object" << std::endl;
            const pdb::UseTemporaryAllocationBlock myBlock{objectSize};
            PDB_COUT << "DataProxy: memory block allocated" << std::endl;
            bool success;
            pdb::Handle<pdb::StorageBytesPinned> ack =
                this->communicator->getNextObject<pdb::StorageBytesPinned>(success, errMsg);
            std::cout << "Received pdb::StorageBytesPinned message" << std::endl;
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                sleep(numTries + 1);
                return pinBytes(dbId, typeId, setId, sizeOfBytes, bytes, needMem, numTries + 1);
            }
            if (ack->getSharedMemOffset() == 0) {
                return false;
            }
            void* dest = this->shm->getPointer(ack->getSharedMemOffset());
            memcpy(dest, bytes, sizeOfBytes);
            return success;
        }
    } else {

        // create a PinBytes object
        {
            pdb::Handle<pdb::StoragePinBytes> msg = pdb::makeObject<pdb::StoragePinBytes>();
            msg->setNodeID(this->nodeId);
            msg->setDatabaseID(dbId);
            msg->setUserTypeID(typeId);
            msg->setSetID(setId);
            msg->setSizeOfBytes(sizeOfBytes);
            // send the message out
            if (!this->communicator->sendObject<pdb::StoragePinBytes>(msg, errMsg)) {
                cout << "Sending object failure: " << errMsg << "\n";
                sleep(numTries + 1);
                return pinBytes(dbId, typeId, setId, sizeOfBytes, bytes, needMem, numTries + 1);
            }
        }
        // receive the PageBytes object
        {
            bool success;
            pdb::Handle<pdb::StorageBytesPinned> ack =
                this->communicator->getNextObject<pdb::StorageBytesPinned>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                sleep(numTries + 1);
                return pinBytes(dbId, typeId, setId, sizeOfBytes, bytes, needMem, numTries + 1);
            }
            if (ack->getSharedMemOffset() == 0) {
                return false;
            }
            void* dest = this->shm->getPointer(ack->getSharedMemOffset());
            memcpy(dest, bytes, sizeOfBytes);
            return success;
        }
    }
}

bool DataProxy::pinTempPage(
    SetID setId, PageID pageId, PDBPagePtr& page, bool needMem, int numTries) {
    return pinUserPage(this->nodeId, 0, 0, setId, pageId, page, needMem, numTries);
}

bool DataProxy::pinUserPage(NodeID nodeId,
                            DatabaseID dbId,
                            UserTypeID typeId,
                            SetID setId,
                            PageID pageId,
                            PDBPagePtr& page,
                            bool needMem,
                            int numTries) {
    if (numTries == MAX_RETRIES) {
        return false;
    }
    if (numTries > 0) {
        logger->error(std::string("DataProxy: pinUserPage with numTries=") +
                      std::to_string(numTries));
    }
    std::string errMsg;
    if (this->communicator->isSocketClosed() == true) {
        std::cout << "ERROR in DataProxy: connection is closed" << std::endl;
        logger->error("DataProxy: connection is closed, to reconnect");
        if (communicator->reconnect(errMsg)) {
            std::cout << errMsg << std::endl;
            logger->error(std::string("DataProxy: reconnect failed with errMsg") + errMsg);
            return false;
        }
    }

    if (nodeId != this->nodeId) {
        this->logger->writeLn(
            "DataProxy: We do not support to load pages from "
            "remote node for the time being.");
        return pinUserPage(nodeId, dbId, typeId, setId, pageId, page, needMem, numTries + 1);
    }

    if (needMem == true) {
        // create a PinPage object
        {
            const pdb::UseTemporaryAllocationBlock myBlock{1024};
            pdb::Handle<pdb::StoragePinPage> msg = pdb::makeObject<pdb::StoragePinPage>();
            msg->setNodeID(nodeId);
            msg->setDatabaseID(dbId);
            msg->setUserTypeID(typeId);
            msg->setSetID(setId);
            msg->setPageID(pageId);
            msg->setWasNewPage(false);

            // send the message out
            if (!this->communicator->sendObject<pdb::StoragePinPage>(msg, errMsg)) {
                cout << "Sending object failure: " << errMsg << "\n";
                return pinUserPage(
                    nodeId, dbId, typeId, setId, pageId, page, needMem, numTries + 1);
            }
        }

        // receive the PagePinned object
        {
            size_t objectSize = this->communicator->getSizeOfNextObject();
            if (objectSize == 0) {
                std::cout << "Receiveing ack failure" << std::endl;
                return pinUserPage(
                    nodeId, dbId, typeId, setId, pageId, page, needMem, numTries + 1);
            }
            const pdb::UseTemporaryAllocationBlock myBlock{objectSize};
            bool success;
            pdb::Handle<pdb::StoragePagePinned> ack =
                this->communicator->getNextObject<pdb::StoragePagePinned>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return pinUserPage(
                    nodeId, dbId, typeId, setId, pageId, page, needMem, numTries + 1);
            }
            char* dataIn = (char*)this->shm->getPointer(ack->getSharedMemOffset());
            page = make_shared<PDBPage>(dataIn, ack->getSharedMemOffset(), 0);
            page->setPinned(true);
            page->setDirty(false);
            return success;
        }
    } else {
        // create a PinPage object
        {
            pdb::Handle<pdb::StoragePinPage> msg = pdb::makeObject<pdb::StoragePinPage>();
            msg->setNodeID(nodeId);
            msg->setDatabaseID(dbId);
            msg->setUserTypeID(typeId);
            msg->setSetID(setId);
            msg->setPageID(pageId);
            msg->setWasNewPage(false);

            // send the message out
            if (!this->communicator->sendObject<pdb::StoragePinPage>(msg, errMsg)) {
                cout << "Sending object failure: " << errMsg << "\n";
                return pinUserPage(
                    nodeId, dbId, typeId, setId, pageId, page, needMem, numTries + 1);
            }
        }

        // receive the PagePinned object
        {
            bool success;
            pdb::Handle<pdb::StoragePagePinned> ack =
                this->communicator->getNextObject<pdb::StoragePagePinned>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return pinUserPage(
                    nodeId, dbId, typeId, setId, pageId, page, needMem, numTries + 1);
            }
            char* dataIn = (char*)this->shm->getPointer(ack->getSharedMemOffset());
            page = make_shared<PDBPage>(dataIn, ack->getSharedMemOffset(), 0);
            page->setPinned(true);
            page->setDirty(false);
            return success;
        }
    }
}


bool DataProxy::unpinTempPage(SetID setId, PDBPagePtr page, bool needMem, int numTries) {
    return unpinUserPage(this->nodeId, 0, 0, setId, page, needMem, numTries);
}

bool DataProxy::unpinUserPage(NodeID nodeId,
                              DatabaseID dbId,
                              UserTypeID typeId,
                              SetID setId,
                              PDBPagePtr page,
                              bool needMem,
                              int numTries) {
    if (numTries == MAX_RETRIES) {
        return false;
    }
    if (numTries > 0) {
        logger->error(std::string("DataProxy: unpinUserPage with numTries=") +
                      std::to_string(numTries));
    }
    std::string errMsg;
    if (this->communicator->isSocketClosed() == true) {
        std::cout << "ERROR in DataProxy: connection is closed" << std::endl;
        logger->error("DataProxy: connection is closed, to reconnect");
        if (communicator->reconnect(errMsg)) {
            std::cout << errMsg << std::endl;
            logger->error(std::string("DataProxy: reconnect failed with errMsg") + errMsg);
            return false;
        }
    }

    if (needMem == true) {
        // create a UnpinPage object
        {
            pdb::UseTemporaryAllocationBlock myBlock{2048};

            pdb::Handle<pdb::StorageUnpinPage> msg = pdb::makeObject<pdb::StorageUnpinPage>();

            msg->setNodeID(nodeId);
            msg->setDatabaseID(dbId);
            msg->setUserTypeID(typeId);
            msg->setSetID(setId);
            msg->setPageID(page->getPageID());

            if (page->isDirty() == true) {
                msg->setWasDirty(true);
            } else {
                msg->setWasDirty(false);
            }

            // send the message out
            if (!this->communicator->sendObject<pdb::StorageUnpinPage>(msg, errMsg)) {
                std::cout << "Sending StorageUnpinPage object failure: " << errMsg << "\n";
                logger->error(std::string("Sending StorageUnpinPage object failure:") + errMsg);
                return unpinUserPage(nodeId, dbId, typeId, setId, page, needMem, numTries + 1);
            }
        }

        // receive the Ack object
        {
            size_t objectSize = this->communicator->getSizeOfNextObject();
            if (objectSize == 0) {
                std::cout << "receive ack failure" << std::endl;
                return unpinUserPage(nodeId, dbId, typeId, setId, page, needMem, numTries + 1);
            }
            pdb::UseTemporaryAllocationBlock myBlock{objectSize};
            bool success;
            pdb::Handle<pdb::SimpleRequestResult> ack =
                this->communicator->getNextObject<pdb::SimpleRequestResult>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return unpinUserPage(nodeId, dbId, typeId, setId, page, needMem, numTries + 1);
            }
            return success && (ack->getRes().first);
        }
    } else {
        // create a UnpinPage object
        {
            pdb::Handle<pdb::StorageUnpinPage> msg = pdb::makeObject<pdb::StorageUnpinPage>();

            msg->setNodeID(nodeId);
            msg->setDatabaseID(dbId);
            msg->setUserTypeID(typeId);
            msg->setSetID(setId);
            msg->setPageID(page->getPageID());

            if (page->isDirty() == true) {
                msg->setWasDirty(true);
            } else {
                msg->setWasDirty(false);
            }

            // send the message out
            if (!this->communicator->sendObject<pdb::StorageUnpinPage>(msg, errMsg)) {
                std::cout << "Sending StorageUnpinPage object failure: " << errMsg << "\n";
                return unpinUserPage(nodeId, dbId, typeId, setId, page, needMem, numTries + 1);
            }
        }

        // receive the Ack object
        {
            bool success;
            pdb::Handle<pdb::SimpleRequestResult> ack =
                this->communicator->getNextObject<pdb::SimpleRequestResult>(success, errMsg);
            if (ack == nullptr) {
                cout << "Receiving ack failure:" << errMsg << "\n";
                return unpinUserPage(nodeId, dbId, typeId, setId, page, needMem, numTries + 1);
            }
            return success && (ack->getRes().first);
        }
    }
}

PageScannerPtr DataProxy::getScanner(int numThreads) {
    std::string errMsg;
    if (this->communicator->isSocketClosed() == true) {
        std::cout << "ERROR in DataProxy.getScanner: connection is closed" << std::endl;
        if (communicator->reconnect(errMsg)) {
            std::cout << errMsg << std::endl;
            return nullptr;
        }
    }
    if (numThreads <= 0) {
        return nullptr;
    }
    int scannerBufferSize;
    if (this->shm->getShmSize() / (DEFAULT_PAGE_SIZE) > 16) {
        scannerBufferSize = 3;
    } else {
        scannerBufferSize = 1;
    }
    PageScannerPtr scanner = make_shared<PageScanner>(
        this->communicator, this->shm, this->logger, numThreads, scannerBufferSize, this->nodeId);
    return scanner;
}

bool DataProxy::printStats(bool needMem) {
    std::string errMsg;
    if (this->communicator->isSocketClosed() == true) {
        std::cout << "ERROR in DataProxy.getScanner: connection is closed" << std::endl;
        if (communicator->reconnect(errMsg)) {
            std::cout << errMsg << std::endl;
            return false;
        }
    }

    pdb::Handle<pdb::StorageGetStats> msg = nullptr;

    if (needMem) {
        pdb::UseTemporaryAllocationBlock myBlock{2048};
        msg = pdb::makeObject<pdb::StorageGetStats>();
        if (!this->communicator->sendObject<pdb::StorageGetStats>(msg, errMsg)) {
            std::cout << "Sending StorageGetStats object failure: " << errMsg << "\n";
        }

        bool success;
        pdb::Handle<pdb::SimpleRequestResult> ack =
            this->communicator->getNextObject<pdb::SimpleRequestResult>(success, errMsg);
        if (ack == nullptr) {
            cerr << "Receiving ack failure:" << errMsg << "\n";
        }
        return success && (ack->getRes().first);
    } else {
        msg = pdb::makeObject<pdb::StorageGetStats>();
        if (!this->communicator->sendObject<pdb::StorageGetStats>(msg, errMsg)) {
            std::cout << "Sending StorageGetStats object failure: " << errMsg << "\n";
        }

        bool success;
        pdb::Handle<pdb::SimpleRequestResult> ack =
            this->communicator->getNextObject<pdb::SimpleRequestResult>(success, errMsg);
        if (ack == nullptr) {
            cerr << "Receiving ack failure:" << errMsg << "\n";
        }
        return success && (ack->getRes().first);

    }
}


#endif
