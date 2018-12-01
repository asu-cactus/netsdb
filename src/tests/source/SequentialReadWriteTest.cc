/*
 *  SequentialReadWrite.cc
 *
 *  Created on: Dec 1, 2018
 *      Author: Jia
 */

#ifndef SEQUENTIAL_READ_WRITE_CC
#define SEQUENTIAL_READ_WRITE_CC

#include <cstddef>
#include <iostream>
#include <vector>
#include <chrono>
#include <ctime>
#include <string>
#include "PangeaStorageServer.h"
#include "TempSet.h"
#include "PDBPage.h"
#include "PageIterator.h"
#include "VariableSizeObjectIterator.h"
#include <fcntl.h>
#include <stdio.h>
#include <memory>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <thread>


#define KB 1024
#define MB (1024 * KB)

using namespace pdb;


void  bufferPoolScan(PageIteratorPtr iter) {
    time_t iterationStart = time(0);
    char * curPointer;
    size_t size;
    int i;
    int curSum = 0;
    int numScannedPages = 0;
    while (iter->hasNext()) {
        PDBPagePtr page = iter->next();
        if(page != nullptr) {
            VariableSizeObjectIteratorPtr objectIter = make_shared<VariableSizeObjectIterator>(page);
            while ((curPointer = objectIter->next(size))!=nullptr) {
                curSum = 0;
                for (i =0; i < size; i ++) {
                    curSum ++;
                    curSum += (int)(*((char *)(curPointer+i)));
                }
            }
            numScannedPages ++;
            page->decRefCount();
         }
   }
   time_t iterationEnd = time(0);
   cout << "scanned "<<numScannedPages <<"\n";
   cout << "iteration time:"<<iterationEnd-iterationStart<<"\n";

   cout << curSum << "\n";

}


int main(int argc, char* argv[]) {

    if(argc < 6) {
       std::cerr << "expect at least 5 arguments: numMillionsObjects, sizeVariance, sizeAvg, numThreads, pageSizeInMB\n";
       exit(-1);
    }
    srand((unsigned int)(time(NULL)));

    int numMillionObjects = atoi(argv[1]);
    int sizeVariance = atoi(argv[2]);
    int sizeAvg = atoi(argv[3]);
    int numThreads = atoi(argv[4]);
    int pageSizeInMB = atoi(argv[5]);
    DurabilityType durabilityType = CacheThrough;
    //DurabilityType durabilityType = TryCache;
    ConfigurationPtr conf = make_shared<Configuration>();
    PDBLoggerPtr logger = make_shared<PDBLogger>(conf->getLogFile());
    SharedMemPtr shm = make_shared<SharedMem>(conf->getShmSize(), logger);
    PDBWorkerQueuePtr myWorkers = make_shared<PDBWorkerQueue>(logger, conf->getMaxConnections());
    std::shared_ptr<PangeaStorageServer> storage = make_shared<PangeaStorageServer>(shm, myWorkers, logger, conf, false);
    storage->startFlushConsumerThreads();
    SetID setId;
    storage->addTempSet("SequentialReadWrite", setId);
    TempSetPtr set = storage->getTempSet(setId);
    storage->getCache()->pin(set,MRU,Write);
    set->setDurabilityType(durabilityType);
    int curSize;
    char * curPointer;

    std::cout << "writing\n" << std::endl;
    time_t bufferPoolAllocationStart = time(0);
    for (int i=0; i<numMillionObjects; i++) {
        for (int j=0; j<1000000; j++) {
             curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
             curPointer = (char *)set->getNewBytes(curSize);
             memset(curPointer, 0, curSize);
        }
    }
    time_t bufferPoolAllocationEnd = time(0);
    std::cout << "BufferPool allocation time: " << bufferPoolAllocationEnd - bufferPoolAllocationStart << std::endl;
    set->pin(MRU, Read);

    std::cout << "start scanning..." << std::endl;
    time_t bufferPoolScanStart = time(0);
    int iteration = 5;
    while (iteration > 0) {
           set->setPinned(true);
           vector<PageIteratorPtr> * iterators = set->getIterators();
           numThreads = iterators->size();

           std::thread* t = new std::thread[numThreads];
           size_t size;
           for (int i = 0; i < numThreads; i ++ ) {
               t[i] = std::thread(bufferPoolScan, iterators->at(i));
           }
           for (int i = 0; i < numThreads; i ++ ) {
               t[i].join();
           }
           set->setPinned(false);
           iteration --;
           delete[] t;        
    }

    time_t bufferPoolScanEnd = time(0);
    std::cout << "BufferPool scan time: " << bufferPoolScanEnd - bufferPoolScanStart << std::endl;
    storage->stopFlushConsumerThreads();

}

#endif
