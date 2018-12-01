/*
 * File: VariableSizeSmallPageAllocator.cc
 * Author: Jia
 *
 * Created on Apr 5, 2016, 21:35 PM
 */

#ifndef VARIABLESIZESMALLPAGEALLOCATOR_CC
#define VARIABLESIZESMALLPAGEALLOCATOR_CC

#include "VariableSizeSmallPageAllocator.h"

VariableSizeSmallPageAllocator::VariableSizeSmallPageAllocator( PDBBackEndServer * server, SetID outputTempSetId, PDBLoggerPtr logger) {
    this->server = server;
    this->outputTempSetId = outputTempSetId;
    this->logger = logger;
    string errMsg;
    BackEndCommunicatorPtr communicator = make_shared<BackEndCommunicator>();
    communicator->connectToInternetServer(logger, server->getConf()->getPort(), "localhost", errMsg, server->getBufferPool());
    this->proxy = make_shared<DataProxy>(server->getNodeId(), communicator, server->getSharedMem(), logger);
    this->numPages = 0;
    this->curLargePage = nullptr;
    this->fullLargePages = new vector<PageHandlePtr>();
    this->unpinPageIfFull = true;
    this->numKeys = 0;
}

VariableSizeSmallPageAllocator::~VariableSizeSmallPageAllocator() {

}

void VariableSizeSmallPageAllocator::unpinCurLargePage() {
   this->logger->writeLn("VariableSizeSmallPageAllocator: unpinCurLargePage()");
   if (this->curLargePage == nullptr) {
       return;
   }
   //this->logger->writeLn("VariableSizeSmallPageAllocator: clearSmallPages()");
   this->curLargePage->clearSmallPages();
   //this->logger->writeLn("VariableSizeSmallPageAllocator: create handle");
   PageHandlePtr handle = make_shared<PageHandle>(this->proxy, this->curLargePage->getPage());
   //this->logger->writeLn("VariableSizeSmallPageAllocator: add handle to list");
   this->fullLargePages->push_back (handle);
   //this->logger->writeLn("VariableSizeSmallPageAllocator: to unpin a handle");
   //To unpin current large page
   handle->unpin();
   this->curLargePage = nullptr;
}

PageHandlePtr VariableSizeSmallPageAllocator::getCurPageHandle() {
   if (this->curLargePage == nullptr) {
       return nullptr;
   }
   return make_shared<PageHandle>(this->proxy, this->curLargePage->getPage());
}

vector<PageHandlePtr> * VariableSizeSmallPageAllocator::getFullLargePages()  { 
   return this->fullLargePages; 
}

AggregationSmallPagePtr VariableSizeSmallPageAllocator::getNewAggregationSmallPage(char * bytes, size_t size) {
    AggregationSmallPagePtr smallPage = nullptr;
    if(curLargePage != nullptr) {
       smallPage = curLargePage->addAggregationSmallPage(bytes, size);
       if(smallPage == nullptr) {
           //unpin the largePage
           if(this->unpinPageIfFull == true) {
               this->unpinCurLargePage();
           } else {
               this->fullLargePages->push_back(make_shared<PageHandle>(this->proxy, this->curLargePage->getPage()));
           }
       }
    }

    if (smallPage == nullptr) {
        curLargePage = this->getNewLargePage();
        //add new smallPage to largePage
        smallPage = curLargePage->addAggregationSmallPage(bytes, size);
    }
    
    return smallPage;
}


void * VariableSizeSmallPageAllocator::getNewBytes(size_t size) {
    void * newBytes = nullptr;
    if(curLargePage != nullptr) {
       newBytes = curLargePage->addBytes(size);
       if(newBytes == nullptr) {
           //unpin the largePage
           if(this->unpinPageIfFull == true) {
               this->unpinCurLargePage();
           } else {
               this->fullLargePages->push_back(make_shared<PageHandle>(this->proxy, this->curLargePage->getPage()));
           }

       }
    }

    if (newBytes == nullptr) {
        curLargePage = this->getNewLargePage();
        //add new smallPage to largePage
        newBytes = curLargePage->addBytes(size);
    }

    return newBytes;

}

LargePageVariablePtr VariableSizeSmallPageAllocator::getNewLargePage() {
   PDBPagePtr page = nullptr;
   LargePageVariablePtr largePage = nullptr;
   if(proxy->addTempPage(this->outputTempSetId, page) != false) {
      if(page != nullptr) {
          largePage = make_shared<LargePageWithVariableSizeSmallPages>(page);
          this->numPages ++;
      }
   }
   return largePage; 

}


#endif
