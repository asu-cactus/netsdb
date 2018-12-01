/*
 * File: FixedSizeSmallPageAllocator.cc
 * Author: Jia
 *
 * Created on Apr 5, 2016, 13:47 PM
 */

#ifndef FIXEDSIZESMALLPAGEALLOCATOR_CC
#define FIXEDSIZESMALLPAGEALLOCATOR_CC

#include "DataProxy.h"
#include "PDBLogger.h"
#include "PDBPage.h"
#include "FixedSizeSmallPageAllocator.h"
#include "Configuration.h"

FixedSizeSmallPageAllocator::FixedSizeSmallPageAllocator(PDBBackEndServer* server, SetID outputTempSetId, PDBLoggerPtr logger) {
    this->outputTempSetId = outputTempSetId;
    //cout << "allocator created with setId="<<this->outputTempSetId<<"\n";
    this->isLocal = false;
    this->curLargePage = nullptr;
    this->server = server;
    this->fullLargePages = new vector<PageHandlePtr>(); 
    this->numPages = 0;
    this->pageUsedUp = false;
    string errMsg;
    server->getLogger()->writeLn("FixedSizeSmallPageAllocator: create a communicator for pin and unpin large pages\n");
    BackEndCommunicatorPtr communicator = make_shared<BackEndCommunicator>();
    communicator->connectToInternetServer(logger, server->getConf()->getPort(), "localhost", errMsg, server->getBufferPool());
    this->proxy = make_shared<DataProxy>(server->getNodeId(), communicator, server->getSharedMem(), logger);
    pthread_mutex_init(&allocatorLock, nullptr);

}

FixedSizeSmallPageAllocator::FixedSizeSmallPageAllocator(PDBStoragePtr storage, SetID outputTempSetId, PDBLoggerPtr logger) {
     this->outputTempSetId = outputTempSetId;
     this->isLocal = true;
     this->curLargePage = nullptr;
     this->storage = storage;
     this->fullLargePagesLocal = new vector<PDBPagePtr> ();
     this->numPages = 0;
     this->pageUsedUp = false;
     this->tempSet = storage->getTempSet(outputTempSetId);
     pthread_mutex_init(&allocatorLock, nullptr);
}


FixedSizeSmallPageAllocator::~FixedSizeSmallPageAllocator() {
     this->unpinCurLargePage(); 
     if(isLocal == false) {
         this->fullLargePages->clear();
         delete this->fullLargePages;
     } else {
         this->fullLargePagesLocal->clear();
         delete this->fullLargePagesLocal;
     }
}


void FixedSizeSmallPageAllocator::unpinCurLargePage() {
     //cout << "FixedSizeSmallPageAllocator: to unpin CurLargePage...\n";
     if(this->curLargePage == nullptr) {
         return;
     }
     if(this->isLocal == false) {
         PageHandlePtr handle = make_shared<PageHandle>(this->proxy, this->curLargePage->getPage());
         this->fullLargePages->push_back(handle);
         handle->unpin();
         this->curLargePage = nullptr;
     }
     else {
         this->curLargePage->getPage()->decRefCount();
         //this->storage->getCache()->evictPage(this->curLargePage->getPage());
         this->curLargePage = nullptr;
     }

}

LargePageFixedPtr FixedSizeSmallPageAllocator::reuseCurPage() {
    PDBPagePtr pageInReturn = nullptr;
    PDBPagePtr pageInFlush = nullptr;
    LargePageFixedPtr pageToReturn = nullptr;
    if(this->isLocal == false) {
        //todo
        return nullptr;
    } else {
        pageInFlush = this->curLargePage->getPage();
        pageInFlush->decRefCount();
        //cout << "to evict page with pageId="<<pageInFlush->getPageID()<<", setId="<<pageInFlush->getSetID()<<"\n";
        this->storage->getCache()->evictPage(pageInFlush);
        pageInReturn = tempSet->addPageByPageInFlush(pageInFlush);
        if(pageInReturn != nullptr) {
           size_t smallPageSize = storage->getConf()->getSmallPageSize();
           pageToReturn = make_shared<LargePageWithFixedSizeSmallPages>(pageInReturn, smallPageSize); 
        }
   }
   return pageToReturn;
}


PageHandlePtr FixedSizeSmallPageAllocator::getCurPageHandle() {
     if (this->curLargePage == nullptr) {
         return nullptr;
     }
     return make_shared<PageHandle>(this->proxy, this->curLargePage->getPage());
}

PDBPagePtr FixedSizeSmallPageAllocator::getCurPage() {
     return this->curLargePage->getPage();
}



vector<PageHandlePtr> * FixedSizeSmallPageAllocator::getFullLargePages() { 
    return this->fullLargePages; 
}

vector<PDBPagePtr> * FixedSizeSmallPageAllocator::getFullLargePagesLocal() {
    return this->fullLargePagesLocal;
}

LargePageFixedPtr FixedSizeSmallPageAllocator::getNewLargePage() {
   //cout << "to get new large page for allocator:" << this->outputTempSetId << "\n";
   PDBPagePtr page = nullptr;
   LargePageFixedPtr largePage = nullptr;
   if(isLocal == false) {
       if(proxy->addTempPage(this->outputTempSetId, page) == false) {
           return nullptr;
       }
   } else {
       //TempSetPtr tempSet = storage->getTempSet(outputTempSetId);
       page = tempSet->addPage();
       //cout << "Got new page with setId="<<this->outputTempSetId<<", pageId="<<page->getPageID()<<"\n";
   }
   if(page != nullptr) {
       size_t smallPageSize;
       if(isLocal == false) {
           smallPageSize = server->getConf()->getSmallPageSize();
       } else {
           smallPageSize = storage->getConf()->getSmallPageSize();
       }
       largePage = make_shared<LargePageWithFixedSizeSmallPages>(page, smallPageSize);
       // largePage = make_shared<LargePageWithFixedSizeSmallPages>(page, SMALL_PAGE_SIZE);
       this->numPages++;
   }
   return largePage;
}

LargePageFixedPtr FixedSizeSmallPageAllocator::tryGetNewLargePage() {
   //cout << "to get new large page for allocator:" << this->outputTempSetId << "\n";
   PDBPagePtr page = nullptr;
   LargePageFixedPtr largePage = nullptr;
   if(isLocal == false) {
       if(proxy->addTempPage(this->outputTempSetId, page) == false) {
           return nullptr;
       }
   } else {
       //TempSetPtr tempSet = storage->getTempSet(outputTempSetId);
       page = tempSet->tryAddPage();
       //cout << "Got new page with setId="<<this->outputTempSetId<<", pageId="<<page->getPageID()<<"\n";
   }
   if(page != nullptr) {
       size_t smallPageSize;
       if(isLocal == false) {
           smallPageSize = server->getConf()->getSmallPageSize();
       } else {
           smallPageSize = storage->getConf()->getSmallPageSize();
       }
       largePage = make_shared<LargePageWithFixedSizeSmallPages>(page, smallPageSize);
       // largePage = make_shared<LargePageWithFixedSizeSmallPages>(page, SMALL_PAGE_SIZE);
       this->numPages++;
   }
   return largePage;
}



SmallPagePtr FixedSizeSmallPageAllocator::getNewSmallPage() {
    //cout << "to get new small page for allocator:" << this->outputTempSetId << "\n";
    SmallPagePtr smallPage = nullptr;
    pthread_mutex_lock(&allocatorLock);
    //cout << "got lock for allocator:"<<this->outputTempSetId<<"\n";
    if((curLargePage != nullptr)&&(this->pageUsedUp==false)) {
       smallPage = curLargePage->addSmallPage();
       //cout << "after adding small page for allocator:"<<this->outputTempSetId<<"\n";
       if(smallPage != nullptr) {
          if(smallPage->getSize() < DEFAULT_SMALL_PAGE_SIZE) {
              //cout << this->outputTempSetId<<": page is used up!\n";
              this->pageUsedUp = true;
          } 
       } else {
          //cout << this->outputTempSetId<<": page is used up!\n";
          this->pageUsedUp = true;
       }
   }

    if ((smallPage == nullptr) && (this->pageUsedUp == false) && (this->curLargePage == nullptr)) {
        if((curLargePage = this->tryGetNewLargePage())==nullptr) {
            //cout << "no page to be allocated from buffer pool!\n";
            exit(-1);
        } else {
            //storage->getCache()->removeLocalitySetFromPriorityList(tempSet, TransientLifetimeNotEndedShuffleData);
            //storage->getCache()->addLocalitySetToPriorityList(tempSet, TransientLifetimeNotEndedShuffleData);
        }
    }

    //cout << "to check whether small page is null for allocator:"<<this->outputTempSetId<<"\n";
    if ((smallPage == nullptr)&&(this->pageUsedUp==true)) {
        //unpin the largePage
        //cout << "to unpin the large page for allocator:"<<this->outputTempSetId<<"\n";
        //this->unpinCurLargePage();
        LargePageFixedPtr page;
        if((page = this->tryGetNewLargePage()) == nullptr) {
            //cout << "to reuse current page in:"<< this->outputTempSetId<<"\n";
            curLargePage = this->reuseCurPage();
            //cout << "page reused in:"<<this->outputTempSetId<<"\n";
        } else {
            this->unpinCurLargePage();
            curLargePage = page;
        }
        //storage->getCache()->removeLocalitySetFromPriorityList(tempSet, TransientLifetimeNotEndedShuffleData);
        //storage->getCache()->addLocalitySetToPriorityList(tempSet, TransientLifetimeNotEndedShuffleData);
        this->pageUsedUp = false;
    }


    if (smallPage==nullptr) {
        //cout << "small page is null for allocator:"<<this->outputTempSetId<<"\n";
        //add new smallPage to largePage
        //cout << "to add small page to newly got large page for allocator:"<<this->outputTempSetId<<"\n";
        if(curLargePage == nullptr) {
            //cout << "can't get large page for allocator:"<<this->outputTempSetId<<"\n";
            smallPage = nullptr;
        } else {
            //cout << "To get small page for allocator:"<<outputTempSetId<<"\n";
            smallPage = curLargePage->addSmallPage();
        }
    }
    //cout << "to unlock for allocator:"<<this->outputTempSetId<<"\n";
    pthread_mutex_unlock(&allocatorLock);
    //cout << "got small page:"<<this->outputTempSetId<<"\n";
    return smallPage;
}

#endif
