/*
 * File: AggregationVirtualBuffer.cc
 * Author: Jia
 *
 * Created on Apr 6, 2016, 14:41 PM
 */

#ifndef AGGREGATIONVIRTUALBUFFER_CC
#define AGGREGATIONVIRTUALBUFFER_CC

#include "AggregationVirtualBuffer.h"
#include "DataTypes.h"
#include "SmallPageAllocatorPageIterator.h"
#include "VariableSizeObjectIterator.h"
#include <stdlib.h>
#include <functional>
#include <string>
using namespace std;

AggregationVirtualBuffer::AggregationVirtualBuffer(int workerId, PDBBackEndServer * server, int numRangePartitions, int iterationId, SetKey setKey, PDBLoggerPtr logger) {
    this->workerId = workerId;
    this->server = server;
    this->logger = logger;
    this->aggregationMap = new unordered_map<string, AggregationSmallPagePtr>();
    this->allocators = new unordered_map<KeyRangeID, VariableSizeSmallPageAllocatorPtr>();
    this->numRangePartitions = numRangePartitions;
    this->iterationId = iterationId;
    this->setKey = setKey;
}

AggregationVirtualBuffer::~AggregationVirtualBuffer() {
    
}

bool AggregationVirtualBuffer::addOrUpdateObjectByBytes(void * bytes, size_t size, int count) {
    AggregationSmallPagePtr smallPage = nullptr;
    VariableSizeSmallPageAllocatorPtr allocator = nullptr;
    string key = this->getKey(bytes, size);
    this->logger->writeLn("key=");
    this->logger->writeLn(key);
    KeyRangeID rangeId = this->getKeyRangeID(key);
    auto iter = aggregationMap->find(key);
    if(iter != aggregationMap->end()) {
        smallPage = iter->second;
        if(smallPage->isUnpinned() == true) {
            (*aggregationMap)[key] = nullptr;
            smallPage = nullptr;
        }
    }
    if(smallPage == nullptr) {
        //a new bucket
        //check whether allocator exists for the key
        auto allocatorIter = allocators->find(rangeId);
        if( allocatorIter  == allocators->end()) {
            //create a new tempset for the key
            string errMsg;
            BackEndCommunicatorPtr communicatorToFrontEnd = make_shared<BackEndCommunicator>();
            communicatorToFrontEnd->connectToInternetServer(logger, server->getConf()->getPort(), "localhost", errMsg, server->getBufferPool());
            DataProxyPtr proxy = make_shared<DataProxy>(server->getNodeId(),communicatorToFrontEnd, this->server->getSharedMem(), logger);
            SetID setId;
            char buffer[50];
            sprintf(buffer, "aggregation_%d_%d_%d", workerId, rangeId, this->iterationId);
            logger->writeLn("to add tempset with name =");
            logger->writeLn(buffer);
            proxy->addTempSet(buffer, setId);
            logger->writeLn("added tempset with setId =");
            logger->writeInt(setId);

            //create a new allocator
            allocator = make_shared<VariableSizeSmallPageAllocator>(server, setId, logger);
            if(allocator == nullptr) {
                return false;
            }
            if(this->numRangePartitions <= 1) {
                //final iteration
                allocator->setUnpinPageIfFull(false);
            }
            //insert allocator to hashmap
            allocators->insert(std::pair<KeyRangeID, VariableSizeSmallPageAllocatorPtr>(rangeId, allocator));
         }
         allocator = allocators->at(rangeId);
         allocator->incNumKeys();
         //add a new small page
         logger->writeLn( "AggregationVirtualBuffer: to get a new small page for the key!" );
         smallPage = allocator->getNewAggregationSmallPage((char *)bytes, size);
         if(smallPage == nullptr) {
             //the large page is full
             return false;
         }
         //insert smallPage to hashmap
         (*aggregationMap)[key] = smallPage;
    }
    //cout << "AggregationVirtualBuffer: update the value!\n";
    smallPage->updateObject(count); 
    return true; 
}


SetID AggregationVirtualBuffer::getTempSetId(KeyRangeID rangeId) {
    VariableSizeSmallPageAllocatorPtr allocator = allocators->at(rangeId);
    if(allocator == nullptr) {
        cout << "AggregationVirtualBuffer: Error: TempSet doesn't exist.\n";
        exit(-1);
    }
    return allocator->getTempSetId();
    
}

string AggregationVirtualBuffer::getKey(void * bytes, size_t size) {
    string ret = string((char*)bytes, size);
    return ret;    
}

KeyRangeID AggregationVirtualBuffer::getKeyRangeID(string key) {
    if(this->numRangePartitions <= 1) {
        return 0;
    }
    size_t keyhash = hash<string>()(key);
    return keyhash%this->numRangePartitions;
}

SetKey AggregationVirtualBuffer::getOutputSetKey() {
    return this->setKey;
}

void AggregationVirtualBuffer::aggregate() {
   //Step 1. Clear partialAggregationMap
   aggregationMap->clear();
   //Step 2. Iterate each KeyRange, and for each KeyRange, do:
   
   for (auto iter = this->allocators->begin(); iter != this->allocators->end();) {
       VariableSizeSmallPageAllocatorPtr allocator = iter->second;

       //Step 2.1 check whether we need to do final aggregation
       if(allocator->getNumPages() == 0) {
           //erase the item
           iter = this->allocators->erase(iter);
       } else if (allocator->getNumPages() == 1) {
           //add that page to output set
           //TODO
           allocator->unpinCurLargePage();
           iter = this->allocators->erase(iter);           
       }
 
       //Step 2.2 add each object in the set to the final aggregationMap
       else {
           //each page represents partial aggregation result
           //needs to aggregate all partial aggregation results
           //Step 2.2.1 create a new AggregationVirtualBuffer instance for final aggregation
           //We assume there are only 2 rounds of iterations
           int numKeysInPartition = allocator->getNumKeys();
           cout << "number of keys in this partition:" << numKeysInPartition<<".\n"; 
           AggregationVirtualBufferPtr curVirtualBuffer = make_shared<AggregationVirtualBuffer> (this->workerId, this->server, 1, this->iterationId+1, this->setKey, this->logger);
           //Step 2.2.2 get a set to scan data
           SmallPageAllocatorPageIteratorPtr pageIterator = make_shared<SmallPageAllocatorPageIterator>(allocator);
           while(pageIterator->hasNext()) {
               PDBPagePtr page = pageIterator->next();
               if(page != nullptr) {
                   VariableSizeObjectIteratorPtr iter = make_shared<VariableSizeObjectIterator>(page);
                   while(iter->hasNext()) {
                      size_t curSize = 0;
                      void * curBytes = iter->nextRaw(curSize);
                      curVirtualBuffer->addOrUpdateObjectByBytes((char*)curBytes+sizeof(int)+sizeof(short), curSize, (*(int*)(curBytes)));
                   }
               }
           }
           //add all generated pages to output set
           //curVirtualBuffer->aggregate();
       }
   }
   
}
#endif 
