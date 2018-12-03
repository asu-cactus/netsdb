#include "PDBDebug.h"
#include "PDBFlushConsumerWork.h"
#include "PageCircularBuffer.h"
#include <chrono>
#include <ctime>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
PDBFlushConsumerWork::PDBFlushConsumerWork(FilePartitionID partitionId,
                                           pdb::PangeaStorageServer* server) {
    this->partitionId = partitionId;
    this->server = server;
    this->isStopped = false;
}

void PDBFlushConsumerWork::stop() {
    this->isStopped = true;
}


void PDBFlushConsumerWork::execute(PDBBuzzerPtr callerBuzzer) {
    PageCircularBufferPtr flushBuffer = this->server->getFlushBuffer();
    PDBPagePtr page;
    SetPtr set = nullptr;
    while (!isStopped) {
        if ((page = flushBuffer->popPageFromHead()) != nullptr) {
            // got a page from flush buffer
            // find the set of the page
          
            std::cout << "Got a page with PageID " << page->getPageID()
                     << " for partition:" << this->partitionId << "\n";
            std::cout << "page dbId=" << page->getDbID() << "\n";
            std::cout << "page typeId=" << page->getTypeID() << "\n";
            std::cout << "page setId=" << page->getSetID() << "\n";
            bool isTempSet = false;
            if ((page->getDbID() == 0) && (page->getTypeID() == 0)) {
                set = this->server->getTempSet(page->getSetID());
                isTempSet = true;
            } else {
                set = this->server->getSet(page->getDbID(), page->getTypeID(), page->getSetID());
                isTempSet = false;
            }
            CacheKey key;
            key.dbId = page->getDbID();
            key.typeId = page->getTypeID();
            key.setId = page->getSetID();
            key.pageId = page->getPageID();
            this->server->getCache()->flushLock();
            if ((set != nullptr) && (page->getRawBytes() != nullptr)) {

                // append the page to the partition
                int ret = set->getFile()->appendPage(this->partitionId, page);
                if (ret < 0) {
                    PDB_COUT << "Can't write page with below info:\n";
                    PDB_COUT << "Got a page with PageID " << page->getPageID()
                             << " for partition:" << this->partitionId << "\n";
                    PDB_COUT << "page dbId=" << page->getDbID() << "\n";
                    PDB_COUT << "page typeId=" << page->getTypeID() << "\n";
                    PDB_COUT << "page setId=" << page->getSetID() << "\n";
                }
                set->lockDirtyPageSet();
                if (isTempSet == false) {                    
                    PDB_COUT << "to write meta" << std::endl;
                    set->getFile()->writeMeta();
                }
                set->removePageFromDirtyPageSet(page->getPageID(), this->partitionId, ret);
                set->unlockDirtyPageSet();
                std::cout << "page with PageID " << page->getPageID()
                         << " appended to partition with PartitionID " << this->partitionId << "\n";
            }
#ifndef UNPIN_FOR_NON_ZERO_REF_COUNT
            if ((page->getRawBytes() != nullptr) && (page->getRefCount() == 0) &&
                (page->isInEviction() == true)) {
#else
            if ((page->getRawBytes() != nullptr) && (page->isInEviction() == true)) {
#endif
                
                // remove the page from cache!
                this->server->getSharedMem()->free(page->getRawBytes() - page->getInternalOffset(),
                                                   page->getSize() + 512);
                PDB_COUT << "internalOffset=" << page->getInternalOffset() << "\n";
                page->setOffset(0);
                page->setRawBytes(nullptr);
            }
// remove the page from cache!
#ifndef UNPIN_FOR_NON_ZERO_REF_COUNT
            if ((page->getRefCount() == 0) && (page->isInEviction() == true)) {
#else
            if (page->isInEviction() == true) {
#endif
                this->server->getCache()->removePage(key);
            }  
            page->setInFlush(false);
            page->setDirty(false);
            
            std::cout << "PDBFlushConsumerWork: page freed from cache" << std::endl;
            this->server->getCache()->flushUnlock();
            this->server->getLogger()->writeLn(
                "PDBFlushConsumerWork: unlocked for flushUnlock()...");
        }
    }
    PDB_COUT << "flushing thread stopped running for partition: " << partitionId << "\n";
}
