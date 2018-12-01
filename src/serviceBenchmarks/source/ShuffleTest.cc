/*
 * Author: Jia
 * Created on May 17, 2016 22:28
 */
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <unordered_map>
#include "VariableSizeObjectIterator.h"
#include "FixedSizeSmallPageAllocator.h"
#include "FixedSizeObjectIterator.h"
#include "PageIterator.h"
#include "OutputVirtualBuffer.h"
#include <sstream>
#include <iostream>
#include <thread>
#include <fcntl.h>
using namespace std;

char * new_string_from_integer(int num) {
   int ndigits = num == 0 ? 1 : (int)log10(num) + 1;
   char * str = (char *) malloc (ndigits + 1);
   sprintf(str, "%d", num);
   return str;
}

int getPartitionId (string const & key, int workerId, int numPartitions) {
   size_t keyhash = hash<string>()(key);
   return (keyhash+workerId) % numPartitions;
}

int getPartitionId (int key, int workerId, int numPartitions) {
   return ((hash<int>()(key)+(workerId+1)))%numPartitions;
}

void fsShuffleWrite(int workerId, int numKeys, int numPartitions, int batchSize, PDBLoggerPtr logger) {
    time_t taskTimeStart=time(0);
    //open file
    char fileName[512];
    vector<FILE *> * files = new vector<FILE *>();
    int i;
    for ( i = 0; i < numPartitions; i ++) {
        sprintf(fileName, "/data/fs/tmp%d_%d", workerId, i);
        FILE * file = fopen(fileName, "w");
        files->push_back(file);
    }

    vector<char *> * buffers = new vector<char *>();
    char * buffer;
    for ( i = 0; i < numPartitions; i ++) {
       buffer = (char *) malloc (batchSize);
       buffers->push_back(buffer);
    }

    vector<int> * offsets = new vector<int>();
    for (i = 0; i < numPartitions; i ++) {
       offsets->push_back(0);
    }

    vector<int> * numObjects = new vector<int>();
    for (i = 0; i < numPartitions; i ++) {
      numObjects->push_back(0);
    }

    int init = 0;
    for (i = 0; i<numPartitions; i++) {
        FILE * file = files->at(i);
        fwrite(&init, sizeof(int), 1, file);
    }

    FILE * curFile;
    char * curBuffer;     
    int curOffset;
    int curNumObjects;
    int value = 0;
    for ( i = 0; i < numKeys; i ++) {
        char * key = new_string_from_integer(i);
        int partitionId = getPartitionId(i, workerId, numPartitions);
        curFile = files->at(partitionId);
        curBuffer = buffers->at(partitionId);
        curOffset = offsets->at(partitionId);
        curNumObjects = numObjects->at(partitionId);
        
        short keySize = strlen(key) + 1;
        size_t totalSize = sizeof(short)+keySize/*+sizeof(int)*/;
        if(totalSize > batchSize-curOffset) {
            fwrite(curBuffer, sizeof(char), curOffset, curFile);
            curOffset = 0;
        } 
        char * cur = curBuffer + curOffset;
        *((short *)cur) = (short)keySize;
        cur = cur + sizeof(short);
        memcpy(cur, key, keySize);
        /*
        cur = cur + keySize;
        *((int *) cur) = value;
        cur = cur + sizeof(int);
        */
        offsets->at(partitionId) = curOffset + totalSize;
        numObjects->at(partitionId) = curNumObjects + 1;
        free(key);
    }
    for (i = 0; i<numPartitions; i++) {
       curFile = files->at(i);
       curBuffer = buffers->at(i);
       curOffset = offsets->at(i);
       curNumObjects = numObjects->at(i);
       fwrite(curBuffer, sizeof(char), curOffset, curFile);
       fseek(curFile, 0, SEEK_SET);
       fwrite(&curNumObjects, sizeof(int), 1, curFile);
    }
    time_t taskTimeEnd = time(0);
    for (i = 0; i < numPartitions; i ++) {
        free(buffers->at(i));
        fclose(files->at(i));
    }

    delete buffers;
    delete files;
    delete offsets;
    cout <<workerId<<":fs task time="<<taskTimeEnd-taskTimeStart<<"\n";
    logger->writeLn("fs task time=");
    logger->writeInt(taskTimeEnd - taskTimeStart);
}

void fsShuffleRead(int partitionId, int numFiles, PDBLoggerPtr logger) {
    int i,j,k;
    char fileName[512];
    int numObjects;
    int curSum = 0;
    time_t taskTimeStart=time(0);
    for(i = 0; i < numFiles; i ++) {
        sprintf(fileName, "/data/fs/tmp%d_%d", i, partitionId);
        FILE * file = fopen(fileName, "r");
        fread(&numObjects, sizeof(int), 1, file);
        cout << "numObjects=" << numObjects << "\n";
        short curSize;
        char curStr[100];
        for(j = 0; j < numObjects; j++) {
            fread(&curSize, sizeof(short), 1, file);
            //cout << "curSize = " << curSize << "\n";
            fread(curStr, sizeof(char), curSize, file);
            //cout << "curStr = " << curStr << "\n"; 
            curSum = 0;
            for (k = 0; k < curSize; k ++) {
                curSum ++;
                curSum += (int)(curStr[i]);
            }
        }
        cout << "curSum = "<< curSum << "\n";
        fclose(file);
    }
    time_t taskTimeEnd=time(0);
    cout << partitionId << ":fs task time="<<taskTimeEnd-taskTimeStart<<"\n";
    logger->writeLn("fs task time=");
    logger->writeInt(taskTimeEnd - taskTimeStart);
}


void pangaeaShuffleWrite(int workerId, int numKeys, int numPartitions, vector<FixedSizeSmallPageAllocatorPtr> * allocators, PDBLoggerPtr logger) {
    time_t taskTimeStart=time(0);
    int i;
    vector<OutputVirtualBufferPtr> * virtualBuffers = new vector<OutputVirtualBufferPtr>();
    for ( i=0; i<numPartitions; i++) {
        OutputVirtualBufferPtr virtualBuffer = make_shared<OutputVirtualBuffer>(i, workerId, allocators->at(i));
        virtualBuffers->push_back(virtualBuffer);
    }
    char * curBytes;
    for ( i=0; i<numKeys; i++) {
        //char * key = new_string_from_integer(i);
        int partitionId = getPartitionId(i, workerId, numPartitions);
        OutputVirtualBufferPtr virtualBuffer = virtualBuffers->at(partitionId);
        int ndigits = i == 0 ? 1 : (int)log10(i) + 1;
        curBytes=virtualBuffer->getBytes(ndigits+1);
        sprintf(curBytes, "%d", i);
    }
    time_t taskTimeEnd = time(0);
    delete virtualBuffers;    
    cout <<workerId<<":Pangaea task time="<<taskTimeEnd-taskTimeStart<<"\n";
    logger->writeLn("Pangaea task time=");
    logger->writeInt(taskTimeEnd - taskTimeStart);
}

void pangaeaShuffleRead(int partitionId, TempSetPtr set, PDBLoggerPtr logger, PDBStoragePtr storage) {
    vector<PageIteratorPtr> * iterators = set->getIterators();
    int numIterators = iterators->size();
    int i,j;
    char * curPointer;
    size_t size;
    int curSum = 0;
    time_t taskTimeStart = time(0);
    size_t smallPageSize = storage->getConf()->getSmallPageSize();
    cout << "smallPageSize = " << smallPageSize << "\n";
    for ( i = 0; i < numIterators; i ++ ) {
        PageIteratorPtr iter = iterators->at(i);
        while (iter->hasNext()) {
            PDBPagePtr page = iter->next();
            //cout << "to process page with pageId=" << page->getPageID() << ", setId="<< page->getSetID() << "\n";
            if ( page != nullptr ) {
                FixedSizeObjectIteratorPtr objectIter = make_shared<FixedSizeObjectIterator>(page, smallPageSize);
                while (( curPointer = objectIter->nextRaw(size)) != nullptr ) {
                   //cout << "size = " << size << "\n";
                   //cout << "str = " << curPointer << "\n";
                   curSum = 0; 
                   for ( j = 0; j < size; j ++ ) {
                       curSum ++;
                       curSum += (int)(curPointer[i]);
                   }
                }

            }
            page->decRefCount();
        }
    }
    time_t taskTimeEnd = time(0);
    cout << partitionId << ": Pangaea task time = " << taskTimeEnd - taskTimeStart << "\n";
    logger->writeLn("Pangaea task time =");
    logger->writeInt(taskTimeEnd - taskTimeStart);
    cout << "curSum = " << curSum << "\n";

}

int main(int argc, char ** argv) {

   if(argc <= 3) {
       cout << "do a simple test...\n";
       char * testStr = new_string_from_integer(100);
       cout << "testStr="<<testStr<<"\n";
       return 1;
   }
   PDBLoggerPtr logger = make_shared<PDBLogger>("ShuffleTest.log");

   int numKeysx1000000 = atoi(argv[1]);
   logger->writeLn("input size=");
   logger->writeInt(numKeysx1000000);

   int numMapThreads = atoi(argv[2]);
   logger->writeLn("numMapThreads=");
   logger->writeInt(numMapThreads);
   int numReadThreads = numMapThreads;

   int numShufflePartitions = atoi(argv[3]);
   logger->writeLn("numShufflePartitions=");
   logger->writeInt(numShufflePartitions);

   int i,j, value = 0;
   if(atoi(argv[4])==0) {
       cout << "Simulate Spark Shuffle in local file system\n";
       logger->writeLn("Simulate Spark Shuffle in local file system\n");
       time_t beforeShuffle = time(0);
       std::thread t[numMapThreads];
       for ( i = 0; i < numMapThreads; i++ ) {
           t[i] = std::thread(fsShuffleWrite, i, numKeysx1000000*1000000, numShufflePartitions, 32*1024, logger);
       }
       for ( i = 0; i < numMapThreads; i++ ) {
           t[i].join();
       }
       time_t afterShuffle = time(0);
       cout << "FS Shuffle time: " << afterShuffle - beforeShuffle << "\n";
       logger->writeLn ("FS Shuffle time=");
       logger->writeInt(afterShuffle - beforeShuffle);
       cout << "Simulate Shuffle Read in local file system\n";
       logger->writeLn("Simulate Shuffle Read in local file system\n");
       time_t beforeRead = time(0);
       std::thread r[numReadThreads];
       for ( i = 0; i < numReadThreads; i++ ) {
           r[i] = std::thread(fsShuffleRead, i, numMapThreads, logger);
       }
       for ( i = 0; i < numReadThreads; i++ ) {
           r[i].join();
       }
       time_t afterRead = time(0);
       cout << "FS Shuffle read time: " << afterRead - beforeRead << "\n";
       logger->writeLn ("FS Shuffle read time=");
       logger->writeInt(afterRead - beforeRead);
   }
   else if(atoi(argv[4])==1){
       cout << "Pangaea Shuffle\n";
       logger->writeLn("Pangaea Shuffle\n");
       DurabilityType durabilityType = CacheThrough;
       ConfigurationPtr conf = make_shared<Configuration>();
       PDBLoggerPtr storageLogger = make_shared<PDBLogger>(conf->getLogFile());
       SharedMemPtr shm = make_shared<SharedMem>(conf->getShmSize(), storageLogger); 
       PDBBufferPoolPtr bufferPool = make_shared<PDBBufferPool> (conf->getRecvBufferSize(), conf->getClientPageSize());
       PDBWorkerQueuePtr myWorkers = make_shared<PDBWorkerQueue>(logger, conf->getMaxConnections());
       PDBStoragePtr storage = make_shared<PDBStorage>(shm, bufferPool, myWorkers, storageLogger, conf);
       storage->startFlushConsumerThreads();      
       //pangaeaShuffleWrite(int workerId, int numKeys, int numPartitions, vector<FixedSizeSmallPageAllocatorPtr> * allocators)
       time_t beforeShuffle = time(0);
       vector<FixedSizeSmallPageAllocatorPtr> * allocators = new vector<FixedSizeSmallPageAllocatorPtr>();
       for (i = 0; i<numShufflePartitions; i++) {
           SetID setId;
           string setName;
           ostringstream convert;
           convert << i;
           setName = convert.str();
           storage->addTempSet(setName, setId);
           TempSetPtr set = storage->getTempSet(setId);
           storage->getCache()->pin(set, MRU, Write);
           FixedSizeSmallPageAllocatorPtr allocator =
                 make_shared<FixedSizeSmallPageAllocator> (storage, setId, logger);
           allocators->push_back(allocator);
           
       }
       std::thread t[numMapThreads];
       for ( i = 0; i < numMapThreads; i++) {
           t[i] = std::thread(pangaeaShuffleWrite, i, numKeysx1000000*1000000, numShufflePartitions, allocators, logger);
       }
       for ( i = 0; i < numMapThreads; i++) {
           t[i].join();
       }
       time_t afterShuffle = time(0);
       //storage->getCache()->unpinAndEvictAllDirtyPages();
       cout << "Pangaea Shuffle time: " << afterShuffle - beforeShuffle << "\n";
       logger->writeLn ("Pangaea Shuffle time=");
       logger->writeInt(afterShuffle - beforeShuffle);
       delete allocators;
       //sleep(5);
      
       cout << "Pangaea Shuffle Read\n";
       logger->writeLn("Pangaea Shuffle Read\n");
       time_t beforeRead = time(0);
       std::thread r[numReadThreads];
       for ( i = 0; i < numReadThreads; i++ ) {
           TempSetPtr set = storage->getTempSet(i);
           set->pin(MRU, Read);
           set->setPinned(true);
           r[i] = std::thread(pangaeaShuffleRead, i, set, logger, storage);
           set->setPinned(false);
           storage->getCache()->unpin(set);
       }
       for ( i = 0; i < numReadThreads; i++ ) {
           r[i].join();
       }
       time_t afterRead = time(0);
       cout << "Pangaea Shuffle read time: " << afterRead - beforeRead << "\n";
       logger->writeLn ("Pangaea Shuffle read time=");
       logger->writeInt(afterRead - beforeRead);
       storage->stopFlushConsumerThreads();
   } else {

   }
   return 0;
}


