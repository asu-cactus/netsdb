/*
 *  AllocationTest.cc
 *
 *  Created on: Apr 29, 2016
 *      Author: Jia
 */



#include "Storage.h"
#include "TempSet.h"
#include "PDBPage.h"
#include "PageIterator.h"
#include "VariableSizeObjectIterator.h"
#include "SlabAllocator.h"
//#include "TachyonSet.h"
//#include "TachyonFileObjectIterator.h"
//#include "RedisSet.h"
//#include "RedisListObjectIterator.h"
//#include "ExternalSetPartitionObjectIterator.h"
#include <fcntl.h>
#include <stdio.h>
#include <memory>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <thread>
//#include <jemalloc/jemalloc.h>
//#include <hdfs/hdfs.h>
//using namespace alluxio;
using namespace std;

//usage:
//#param1: numObjectsx1000000;
//#param2: size variance of objects, and 0 for fixed size;
//#param3: size in average
//#param4: number of threads;
//#param5: allocation strategy: 0 for C-standard malloc and free, with scanning, 1 for memcached slab allocator with scanning, 2 for buffer pool binary data with scanning, 3 for C-standard malloc and free without scanning, 4 for shared memory malloc and free without scanning, 5 for stl allocator without scanning, 6 for memcached slab allocator without scanning, 7 for tachyon allocation and scanning.

//#param6: pageSizex1024


void standardScan(char ** array, int startIndex, int endIndex) {
    int i, j;
    int curSize;
    char * curPointer;
    int curSum = 0;
    for (i = startIndex; i < endIndex; i++) {
               curPointer = array[i];
               curSize = (int)(* ((int *) curPointer));
               curPointer = curPointer + sizeof(int);
               curSum = 0;
               for (j = 0; j < curSize; j++) {
                  curSum ++;
                  curSum += (int)curPointer[j];
               }
     }
     cout << curSum << "\n";


}

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
            while ((curPointer = objectIter->nextRaw(size))!=nullptr) {
                curSum = 0;
                for (i =0; i < size; i ++) {
                    curSum ++;
                    curSum += (int)(curPointer[i]);
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

/*
void tachyonScan(TachyonFileObjectIteratorPtr iter) {
//JNI code can not run in a C++ thread, if the thread hasn't been attached to a JVM
    char buffer[512];
    char * curPointer;
    size_t size = 0;
    int i;
    int curSum = 0;
    while(iter->nextRaw(buffer, size)== true) {
        curPointer = buffer;
        curSum = 0;
        for (i=0; i < size; i ++) {
            curSum ++;
            curSum += (int)(curPointer[i]);
        }
    }
    cout << curSum << "\n";

}
*/
/*
void redisScan(RedisListObjectIteratorPtr iter) {
    char * curPointer;
    size_t size = 0;
    int i;
    int curSum = 0;
    while (iter->nextRaw(&curPointer, size) == true) {
        curSum = 0;
        for (i = 0; i < size; i ++) {
            curSum ++;
            curSum += (int)(curPointer[i]);
        }
    }
    cout << curSum << "\n";
}
*/
int main (int argc, char* argv[])
{
   int numObjectsx1000000 = 10;
   int sizeVariance = 0;
   int sizeAvg = 80;
   int numThreads = 4;
   int allocMode = 0;
   int pageSizex1024 = 64*1024;
   srand((unsigned int)(time(NULL)));

   if(argc < 6) {
       std::cerr << "expect at least 5 arguments: numObjectsx1000000, sizeVariance, sizeAvg, numThreads, allocMode(0 for C-standard malloc and free, with scanning, 1 for OS binary data with scanning, 2 for buffer pool binary data with scanning, 3 for C-standard malloc and free without scanning, 4 for shared memory malloc and free without scanning, 5 for stl allocator without scanning, 6 for memcached slab allocator without scanning, 7 for tachyon allocation and scanning.), pageSizex1024(optional)\n";
       exit(-1);
   }


   numObjectsx1000000 = atoi(argv[1]);
   sizeVariance = atoi(argv[2]);
   sizeAvg = atoi(argv[3]);
   numThreads = atoi(argv[4]);
   allocMode = atoi(argv[5]);
   if(argc == 7) {
       pageSizex1024 = atoi(argv[6]);
   }
   int iteration = 5;


   int i, j, k;
   int curSize;
   char * curPointer;
   int curSum;
   if(allocMode == 0) {
       PDBLoggerPtr logger = make_shared<PDBLogger>("alloc0.log");       
       //C++ standard
       //generate data
       time_t standardAllocationStart = time(0);
       char ** array = (char **) malloc (numObjectsx1000000 * 1000000 * sizeof(char *));
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               char * bytes = (char *) malloc(sizeof(int) + curSize * sizeof(char));
               * ((int *) bytes) = curSize;
               memset(bytes + sizeof(int), 0, curSize);
               array[i*1000000 + j] = bytes;               
           }
       }
       time_t standardAllocationEnd = time(0);
       cout << "Standard allocation time: " << standardAllocationEnd - standardAllocationStart << "\n";
       logger->writeInt(standardAllocationEnd - standardAllocationStart);
       
       //reading data
     
       time_t standardScanStart = time(0);
       int partitionSize = numObjectsx1000000 * 1000000 / numThreads;
       int remainder = numObjectsx1000000 * 1000000 % numThreads;
       int startIndex=0;
       int endIndex=0;
       while (iteration > 0){
           std::thread t[numThreads];
           for (i = 0; i < numThreads; i++) {
              if(i < remainder){
                  startIndex = endIndex;
                  endIndex = startIndex + partitionSize + 1;
              }
              else {
                  startIndex = endIndex;
                  endIndex = startIndex + partitionSize;
              }
              t[i] = std::thread(standardScan, array, startIndex, endIndex);
              //cout << "startIndex=" << startIndex << "\n";
              //cout << "endIndex=" << endIndex << "\n"; 
           }
           for (i = 0; i < numThreads; i ++) {
              t[i].join();
           }
           iteration --;
           endIndex = 0;
       }
       time_t standardScanEnd = time(0);
       cout << "Standard scan time: " << standardScanEnd - standardScanStart << "\n";
       logger->writeLn("\n");
       logger->writeInt(standardScanEnd - standardScanStart);
       //cout << curSum << "\n";
       //deallocating data
       time_t standardDeallocationStart = time(0);
       for (i = 0; i<1000000 * numObjectsx1000000; i++) {
           curPointer = array[i];
           free (curPointer);
       }
       free (array);
       time_t standardDeallocationEnd = time(0);
       cout << "Standard deallocation time: " << standardDeallocationEnd - standardDeallocationStart << "\n";
       logger->writeLn("\n");
       logger->writeInt(standardDeallocationEnd - standardDeallocationStart);
     
   } else if (allocMode == 1) {
          PDBLoggerPtr logger = make_shared<PDBLogger>("alloc0.log");
ConfigurationPtr conf = make_shared<Configuration>();      
 //Slab allocator
       SlabAllocatorPtr allocator = make_shared<SlabAllocator>((size_t)35*(size_t)1024*(size_t)1024*(size_t)1024, false);
       cout << "SlabAllocator Pool Constructed!\n";
       //generate data
       time_t standardAllocationStart = time(0);
       void* array = (void *) malloc (numObjectsx1000000 * 1000000 * sizeof(char *));
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               char * bytes = (char *)allocator->do_slabs_alloc(sizeof(int) + curSize * sizeof(char));
               * ((int *) bytes) = curSize;
               memset(bytes + sizeof(int), 0, curSize);
               char * curPointer = (char *)array +(i*1000000+j)*sizeof(char *);
               *((char **) curPointer) = (char *)bytes;
           }
       }
       time_t standardAllocationEnd = time(0);
       cout << "Slab allocation time: " << standardAllocationEnd - standardAllocationStart << "\n";
       logger->writeInt(standardAllocationEnd - standardAllocationStart);

       //reading data
      //reading data

       time_t standardScanStart = time(0);
       int partitionSize = numObjectsx1000000 * 1000000 / numThreads;
       int remainder = numObjectsx1000000 * 1000000 % numThreads;
       int startIndex=0;
       int endIndex=0;
       while (iteration > 0){
           std::thread t[numThreads];
           for (i = 0; i < numThreads; i++) {
              if(i < remainder){
                  startIndex = endIndex;
                  endIndex = startIndex + partitionSize + 1;
              }
              else {
                  startIndex = endIndex;
                  endIndex = startIndex + partitionSize;
              }
              t[i] = std::thread(standardScan, (char **)array, startIndex, endIndex);
              //cout << "startIndex=" << startIndex << "\n";
              //cout << "endIndex=" << endIndex << "\n"; 
           }
           for (i = 0; i < numThreads; i ++) {
              t[i].join();
           }
           iteration --;
           endIndex = 0;
       }
       time_t standardScanEnd = time(0);
       cout << "Slab scan time: " << standardScanEnd - standardScanStart << "\n";
       logger->writeLn("\n");
       logger->writeInt(standardScanEnd - standardScanStart);
       //cout << curSum << "\n";
       //deallocating data
       time_t standardDeallocationStart = time(0);
       for (i = 0; i<1000000 * numObjectsx1000000; i++) {
           curPointer = ((char**)array)[i];
           allocator->do_slabs_free (curPointer, (int)(* ((int *) curPointer)));
       }
       free(array);
       time_t standardDeallocationEnd = time(0);
       cout << "Slab deallocation time: " << standardDeallocationEnd - standardDeallocationStart << "\n";
       logger->writeLn("\n");
       logger->writeInt(standardDeallocationEnd - standardDeallocationStart);


    
   } else if (allocMode == 2){
       
       //binary data with buffer pool paging
       //generate data
       cout << "running\n";
       DurabilityType durabilityType = CacheThrough;
       //DurabilityType durabilityType = TryCache;       
       ConfigurationPtr conf = make_shared<Configuration>();
       PDBLoggerPtr logger = make_shared<PDBLogger>(conf->getLogFile());
       cout <<"log created\n";
       SharedMemPtr shm = make_shared<SharedMem>(conf->getShmSize(), logger);
       cout << "shm created\n";
       PDBBufferPoolPtr bufferPool = make_shared<PDBBufferPool> (conf->getRecvBufferSize(), conf->getClientPageSize());
       PDBWorkerQueuePtr myWorkers = make_shared<PDBWorkerQueue>(logger, conf->getMaxConnections());
       PDBStoragePtr storage = make_shared<PDBStorage>(shm, bufferPool, myWorkers, logger, conf);
       storage->startFlushConsumerThreads();
       SetID setId;
       storage->addTempSet("testAllocation", setId);
       TempSetPtr set = storage->getTempSet(setId);
       storage->getCache()->pin(set, MRU, Write);
       set->setDurabilityType(durabilityType);
       cout << "writing\n";
       time_t bufferPoolAllocationStart = time(0);
       for (i=0; i< numObjectsx1000000; i ++) {
           for (j=0; j< 1000000; j++) {
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               curPointer = (char *)set->getNewBytes(curSize);
               memset(curPointer, 0, curSize);
           }
       }
       time_t bufferPoolAllocationEnd = time(0);
       cout << "BufferPool allocation time: " << bufferPoolAllocationEnd - bufferPoolAllocationStart << "\n";
       set->pin(MRU, Read); 
       /*
       set->unpinBufferPage();
       if(durabilityType!=CacheThrough) {
           set->flushDirtyPages();
           cout << "flush dirty pages that hasn't been flushed and then sleep for 15 seconds to wait for flushing to finish, flushing time will not be measured for fair comparison.\n";
           sleep (15);
       }*/      
      //reading data
       cout << "start scanning...\n";
       time_t bufferPoolScanStart = time(0);
       while (iteration > 0) {
           set->setPinned(true);
           vector<PageIteratorPtr> * iterators = set->getIterators();
           numThreads = iterators->size();
           
           std::thread t[numThreads];
           size_t size;
           for (i = 0; i < numThreads; i ++ ) {
               t[i] = std::thread(bufferPoolScan, iterators->at(i));
           }
           for (i = 0; i < numThreads; i ++ ) {
               t[i].join();
           }
           /*
           for (i = 0; i < numThreads; i++) {
                bufferPoolScan (iterators->at(i));
           }
           */
           set->setPinned(false);
           iteration --;

       }
       
       time_t bufferPoolScanEnd = time(0);
       cout << "BufferPool scan time: " << bufferPoolScanEnd - bufferPoolScanStart << "\n";
       //cout << curSum << "\n";
       // deallocating

       storage->stopFlushConsumerThreads(); 
            
   } else if (allocMode == 3){
       PDBLoggerPtr logger = make_shared<PDBLogger>("alloc3.log");
       //C++ standard
       //generate data
       time_t standardAllocDeallocStart = time(0);
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               char * bytes = (char *) malloc(curSize * sizeof(char));
               free(bytes);
           }
       }
       time_t standardAllocDeallocEnd = time(0);
       cout << "Standard alloc/dealloc time: " << standardAllocDeallocEnd - standardAllocDeallocStart << "\n";
       logger->writeInt(standardAllocDeallocEnd - standardAllocDeallocStart);


   } else if (allocMode == 4){
       /*
       ConfigurationPtr conf = make_shared<Configuration>();
       PDBLoggerPtr logger = make_shared<PDBLogger>(conf->getLogFile());
       SharedMemPtr shm = make_shared<SharedMem>(conf->getShmSize(), logger);
       time_t shmAllocDeallocStart = time(0);
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               char * bytes = (char *) shm->_malloc_unsafe(curSize * sizeof(char));
               shm->_free_unsafe(bytes);
           }
       }
       time_t shmAllocDeallocEnd = time(0);
       cout << "Shared memory alloc/dealloc time: " << shmAllocDeallocEnd - shmAllocDeallocStart << "\n";
       logger->writeInt(shmAllocDeallocEnd - shmAllocDeallocStart);
       */
   } else if (allocMode == 5){
       PDBLoggerPtr logger = make_shared<PDBLogger>("alloc5.log");
       //STL standard
       //generate data
       std::allocator<char> stlAllocator;
       time_t stlAllocDeallocStart = time(0);
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               char * bytes = stlAllocator.allocate(curSize);
               stlAllocator.deallocate(bytes, curSize);
           }
       }
       time_t stlAllocDeallocEnd = time(0);
       cout << "Stl alloc/dealloc time: " << stlAllocDeallocEnd - stlAllocDeallocStart << "\n";
       logger->writeInt(stlAllocDeallocEnd - stlAllocDeallocStart);
   }
   else if (allocMode == 6){
       
       ConfigurationPtr conf = make_shared<Configuration>();
       PDBLoggerPtr logger = make_shared<PDBLogger>("alloc6.log");
       //Memcached Slab Allocator initialize
       SlabAllocatorPtr slabAllocator = make_shared<SlabAllocator>(conf->getShmSize());
       //generate data
       time_t slabAllocDeallocStart = time(0);
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               char * bytes = (char *) slabAllocator->do_slabs_alloc(curSize * sizeof(char));
               slabAllocator->do_slabs_free(bytes, curSize*sizeof(char));
           }
       }
       time_t slabAllocDeallocEnd = time(0);
       cout << "Slab alloc/dealloc time: " << slabAllocDeallocEnd - slabAllocDeallocStart << "\n";
       logger->writeInt(slabAllocDeallocEnd - slabAllocDeallocStart);

   } else if (allocMode == 7){
       /*
       PDBLoggerPtr logger = make_shared<PDBLogger>("alloc7.log");
       //Tachyon
       TachyonSetPtr tachyonSet = make_shared<TachyonSet>("allocationTest", numThreads);
       //generate data
       time_t tachyonAllocationStart = time(0);
       char value[512];
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               memset(value, 0, curSize);
               tachyonSet->addBytes(j%numThreads, value, curSize);
           }
       }
       time_t tachyonAllocationEnd = time(0);
       cout << "Tachyon allocation time: " << tachyonAllocationEnd - tachyonAllocationStart << "\n";
       logger->writeInt(tachyonAllocationEnd - tachyonAllocationStart);

       time_t tachyonScanStart = time(0); 
       while (iteration > 0) {       
           vector<ExternalSetPartitionObjectIteratorPtr> * iterators = tachyonSet->getIterators();
           TachyonFileObjectIteratorPtr iterator = dynamic_pointer_cast<TachyonFileObjectIterator>(iterators->at(0));
           //jFileInStream inStream = iterator->getInStream();
           char buffer[512];
           char * curPointer;
           size_t size=0;
           int i;
           int curSum = 0;
           while(iterator->nextRaw(buffer, size)== true) {
               curPointer = buffer;
               //cout << "size="<<size<<"\n";
               curSum = 0;
               for (i=0; i < size; i ++) {
                  curSum ++;
                  curSum += (int)(curPointer[i]);
               }
           }
           cout << curSum << "\n";
           delete iterators;
           iteration --;
       }
       time_t tachyonScanEnd = time(0);
       //inStream->close();
       cout << "Tachyon scan time: " << tachyonScanEnd - tachyonScanStart << "\n";
       logger->writeLn("\n");
       logger->writeInt(tachyonScanEnd - tachyonScanStart);
       
        
       tachyonScanStart = time(0);
       while (iteration > 0){
           vector<ExternalSetPartitionObjectIteratorPtr> * iterators = tachyonSet->getIterators();
           std::thread t[numThreads];
           for (i = 0; i < numThreads; i ++ ) {
               cout << "i="<<i<<"\n";
               t[i] = std::thread(tachyonScan, dynamic_pointer_cast<TachyonFileObjectIterator>(iterators->at(i)));
           }
           for (i = 0; i < numThreads; i ++ ) {
               t[i].join();
           }
           delete iterators;
           iteration --;
       }
       tachyonScanEnd = time(0);
       cout << "Tachyon scan time: " << tachyonScanEnd - tachyonScanStart << "\n";
       logger->writeLn("\n");
       logger->writeInt(tachyonScanEnd - tachyonScanStart);
       */
   } else if (allocMode == 8){
       /*
       PDBLoggerPtr logger = make_shared<PDBLogger>("alloc8.log");
       //Redis
       RedisSetPtr redisSet = make_shared<RedisSet>("allocationTest", numThreads, 100000);
       //generate data
       time_t redisAllocationStart = time(0);
       char value[512];
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               memset(value, 0, curSize);
               redisSet->addBytes(j%numThreads, value, curSize);
           }
       }
       time_t redisAllocationEnd = time(0);
       cout << "Redis allocation time: " << redisAllocationEnd - redisAllocationStart << "\n";
       logger->writeInt(redisAllocationEnd - redisAllocationStart);

       time_t redisScanStart = time(0);
       while (iteration > 0) {
           vector<ExternalSetPartitionObjectIteratorPtr> * iterators = redisSet->getIterators();
           RedisListObjectIteratorPtr iterator = dynamic_pointer_cast<RedisListObjectIterator>(iterators->at(0));
           char * curPointer = nullptr;
           size_t size=0;
           int i;
           int curSum = 0;
           while(iterator->nextRaw(&curPointer, size)== true) {
               //cout << "size="<<size<<"\n";
               //cout << "curPointer="<<curPointer<<"\n";
               curSum = 0;
               for (i=0; i < size; i ++) {
                  //cout << "i="<<i<<"\n";
                  curSum ++;
                  //cout << "curSum="<<curSum<<"\n";
                  curSum += (int)(curPointer[i]);
               }
           }
           cout << curSum << "\n";
           delete iterators;
           iteration --;
       }
       time_t redisScanEnd = time(0);
       cout << "Redis scan time: " << redisScanEnd - redisScanStart << "\n";
       logger->writeLn("\n");
       logger->writeInt(redisScanEnd - redisScanStart);


       redisScanStart = time(0);
       while (iteration > 0){
           vector<ExternalSetPartitionObjectIteratorPtr> * iterators = redisSet->getIterators();
           std::thread t[numThreads];
           for (i = 0; i < numThreads; i ++ ) {
               //cout << "i="<<i<<"\n";
               t[i] = std::thread(redisScan, dynamic_pointer_cast<RedisListObjectIterator>(iterators->at(i)));
           }
           for (i = 0; i < numThreads; i ++ ) {
               t[i].join();
           }
           delete iterators;
           iteration --;
       }
       redisScanEnd = time(0);
       cout << "redisScan time: " << redisScanEnd - redisScanStart << "\n";
       logger->writeLn("\n");
       logger->writeInt(redisScanEnd - redisScanStart);
       */
   }
   else if (allocMode == 9) {
       //write to an OS File and scan it later
       int handle = open("/data10/testFile", O_RDWR|O_APPEND|O_CREAT|O_DIRECT, S_IRWXU|S_IRWXO);
       char value[512];
       size_t curSize;
       int i,j,k;
       time_t fsAllocationStart = time(0);
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               write(handle, &curSize, sizeof(size_t));
               memset(value, 0, curSize);
               write(handle, value, curSize);
           }
           
       }
       time_t fsAllocationEnd = time(0);
       cout << "OS filesystem write time:"<<fsAllocationEnd-fsAllocationStart<<"\n";

       close(handle);
       handle = open("/data10/testFile", O_RDWR|O_APPEND|O_CREAT|O_DIRECT, S_IRWXU|S_IRWXO);
       time_t fsScanStart = time(0);
       char * curPointer;
       int curSum = 0;
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               read(handle, &curSize, sizeof(size_t));
               read(handle, value, curSize);
               curPointer = value;
               curSum = 0;
               for (k=0; k<curSize; k++) {
                   curSum ++;
                   curSum += (int)(curPointer[k]);
               }
               
           }

       }
       cout << curSum << "\n";
       time_t fsScanEnd=time(0);
       cout << "OS filesytem read time:"<<fsScanEnd-fsScanStart<<"\n"; 
       close(handle);
   } else if(allocMode == 10) {
       
       FILE * file = fopen("/data/testFile", "w");
       char value[512];
       size_t curSize;
       int i,j,k;
       time_t fsAllocationStart = time(0);
       PDBLoggerPtr logger = make_shared<PDBLogger>("alloc10.log");
       for ( i = 0 ; i < 10*numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 100000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               fwrite(&curSize, sizeof(size_t),1,file);
               memset(value, 0, curSize);
               fwrite(value, sizeof(char), curSize, file);
           }
           if(i%8 == 0) {
               fflush(file);
               fsync(file);
           }
       }
       fflush(file);
       fsync(file);
       fclose(file);
       time_t fsAllocationEnd = time(0);
       cout << "OS filesystem write time:"<<fsAllocationEnd-fsAllocationStart<<"\n";
       logger->writeLn("OS filesystem write time:");
       logger->writeInt(fsAllocationEnd-fsAllocationStart);
       file = fopen("/data/testFile", "r");
       time_t fsScanStart = time(0);
       char * curPointer;
       int curSum = 0;
       while (iteration > 0){
           time_t iterationStart = time(0);
           fseek(file, 0, SEEK_SET);
           for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
               for ( j = 0; j < 1000000; j ++ ) {
                   //get Size
                   fread(&curSize, sizeof(size_t),1, file);
                   fread(value, sizeof(char), curSize, file);
                   curPointer = value;
                   curSum = 0;
                   for (k=0; k<curSize; k++) {
                       curSum ++;
                       curSum += (int)(curPointer[k]);
                   }

               }

            }
            time_t iterationEnd = time(0);
            cout << "iteration time:"<<iterationEnd-iterationStart<<"\n";
            logger->writeLn("OS filesystem iteration time:");
            logger->writeInt(iterationEnd-iterationStart);
            iteration--;
            cout << curSum << "\n";
       }
       time_t fsScanEnd=time(0);
       cout << "OS filesytem read time:"<<fsScanEnd-fsScanStart<<"\n";
       logger->writeLn("OS filesystem read time:");
       logger->writeInt(fsScanEnd-fsScanStart);
       fclose(file);

   } else if(allocMode == 11) {
              
       //write to an HDFS file and scan it later
       //connect to HDFS
       /*
       struct hdfsBuilder * builder = hdfsNewBuilder();
       hdfsBuilderSetNameNode(builder, "localhost");
       hdfsBuilderSetNameNodePort(builder,9000);
       hdfsFS fs = hdfsBuilderConnect(builder);
       hdfsFreeBuilder(builder);
       hdfsFile file = hdfsOpenFile(fs, "/testFile", O_WRONLY, 0, 0, 0);
       //write to HDFS
       char value[512];
       size_t curSize;
       int i,j,k;
       time_t hdfsAllocationStart = time(0);
       for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
           for ( j = 0; j < 1000000; j ++ ) {
               //get Size
               curSize = sizeAvg + (int)((double) sizeVariance * (rand()/double(RAND_MAX)));
               //if(curSize == 0) {
                   //cout << "curSize="<<curSize<<"\n";
               //}
               memset(value, 0, curSize);
               hdfsWrite(fs, file, &curSize, sizeof(size_t));
               hdfsWrite(fs, file, value, curSize);
           }
       }
       hdfsCloseFile(fs, file);
       time_t hdfsAllocationEnd = time(0);
       cout << "HDFS filesystem write time:"<<hdfsAllocationEnd-hdfsAllocationStart<<"\n";  
       //read from HDFS
       time_t hdfsScanStart = time(0);
       char * curPointer;
       int curSum = 0;
       while (iteration > 0){
           time_t iterationStart = time(0);
           file = hdfsOpenFile(fs, "/testFile", O_RDONLY, 0, 0, 0);
           for ( i = 0 ; i < numObjectsx1000000; i ++ ) {
               for ( j = 0; j < 1000000; j ++ ) {
                   //get Size
                   hdfsRead(fs, file, &curSize, sizeof(size_t));
                   hdfsRead(fs, file, value, curSize);
                   curPointer = value;
                   curSum = 0;
                   //cout << "i="<<i<<",j="<<j<<"\n";
                   //cout << "curSize="<<curSize<<"\n";
                   for (k=0; k<curSize; k++) {
                       curSum ++;
                       //cout << "k="<<k<<"\n";
                       curSum += (int)(curPointer[k]);
                   }

               }
               //cout <<i<<": 1000000 objects scanned\n";
            }
            hdfsCloseFile(fs, file);
            time_t iterationEnd = time(0);
            iteration--;
            cout << "iteration time:"<<iterationEnd-iterationStart<<"\n";
            cout << curSum << "\n";
       }
       time_t hdfsScanEnd=time(0);
       cout << "HDFS filesytem read time:"<<hdfsScanEnd-hdfsScanStart<<"\n";
       */
   }
       
}
