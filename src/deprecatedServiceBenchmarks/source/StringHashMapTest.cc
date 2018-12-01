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
#include <hiredis.h>
#include "STLScopedAllocator.h"
#include "STLSlabAllocator.h"
#include "TestVirtualHashMap.h"
#include "TestVirtualHashMapWithSlabAllocator.h"
#include "VariableSizeObjectIterator.h"
#include <iostream>
using namespace std;

string new_string_from_integer(int num) {
   int ndigits = num == 0 ? 1 : (int)log10(num) + 1;
   char * str = (char *) malloc (ndigits + 1);
   sprintf(str, "%d", num);
   string retStr = string(str);
   delete str;
   return retStr;
}

int main(int argc, char ** argv) {

   if(argc <= 3) {
       cout << "do a simple test...\n";
       string testStr = new_string_from_integer(100);
       cout << "testStr="<<testStr<<"\n";
       return 1;
   }
   PDBLoggerPtr logger = make_shared<PDBLogger>("StringHashMap.log");
   int numKeysx1000000 = atoi(argv[1]);
   int numKeysx10 = atoi(argv[1]);
   logger->writeLn("input size=");
   logger->writeInt(numKeysx1000000);
   int i,j, value = 0;
   if(atoi(argv[2])==0) {
       cout << "unordered_map using std::Allocator\n";
       logger->writeLn("STL unordered_map using std::Allocator\n");
       std::unordered_map<string, int>* str_hash=new std::unordered_map<string, int>();
       if(!strcmp(argv[3], "sequentialstring")) {
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 1000000; j++ ) {
                   str_hash->insert(std::unordered_map<string, int>::value_type(new_string_from_integer(i*1000000+j), value));
               }
               cout <<i << ":inserted 1000000 pairs\n";
           }
           time_t after = time(0);
           cout << "std unordered_map sequential string time:"<<after-before<<"\n";
           logger->writeLn("insertion time=");
           logger->writeInt(after-before);
       }
       else if(!strcmp(argv[3], "randomstring")){
           srandom(1);
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 1000000; j++ ) {
                   str_hash->insert(std::unordered_map<string, int>::value_type(new_string_from_integer((int)random()), value));
               }
               cout << i<<":inserted 1000000 pairs\n";
           }
           time_t after = time(0);
           cout << "std unordered_map random string time:"<<after-before<<"\n";
           logger->writeLn("unordered_map insertion time=");
           logger->writeInt(after-before);
       }
       time_t beforeErase = time(0);
       int count = 0;
       i=0;
       for (auto it = str_hash->begin(); it != str_hash->end(); it++) {
          // it = str_hash->erase(it);
          count ++;
          if(count%1000000 == 0) {
             cout <<i<< ":iterated 1000000 key-value pairs\n";
             i++; 
          }
       }
       time_t afterErase = time(0);
       cout << "std unordered_map sequential string scanning time:"<<afterErase - beforeErase<<"\n";
       logger->writeLn("unordered_map iteration time=");
       logger->writeInt(afterErase-beforeErase);
       cout << "count="<<count<<"\n";
   }
   else if(atoi(argv[2])==1){
       cout << "Page-scoped unordered_map using ScopedAllocator\n";
       char * data = (char *) malloc (64*1024*1024*sizeof(char));
       STLScopedAllocator<std::pair<string, int>> allocator = STLScopedAllocator<std::pair<string, int>>((void *)data, 64*1024*1024);
       hash<string> hasher = hash<string>();
       equal_to<string> key_equal=equal_to<string>();
       std::unordered_map<string, int, hash<string>, equal_to<string>,STLScopedAllocator<std::pair<string, int>>>* str_hash = new std::unordered_map<string, int, hash<string>, equal_to<string>,STLScopedAllocator<std::pair<string, int>>> (1040000, hasher, key_equal, allocator);
       if(!strcmp(argv[3], "sequentialstring")) {
           time_t before = time(0);
           for(i = 0; i< numKeysx10; i++) {
               for (j = 0; j < 10; j++ ) {
                  try{
                   str_hash->insert(std::pair<string, int>(new_string_from_integer(i*10+j), value));
                  }
                  catch(std::bad_alloc &e) {
                       cout <<"Caught the exception! Should add a new page, but simply exit now...\n";
                       exit(-1);
                  }
               }
           }
           time_t after = time(0);
           cout << "std unordered_map sequential string time:"<<after-before<<"\n";
       }
       else if(!strcmp(argv[3], "randomstring")){
           srandom(1);
           time_t before = time(0);
           for(i = 0; i< numKeysx10; i++) {
               for (j = 0; j < 10; j++ ) {
                   str_hash->insert(std::pair<string, int>(new_string_from_integer((int)random()), value));
               }
           }
           time_t after = time(0);
           cout << "std unordered_map random string time:"<<after-before<<"\n";
       }
       delete data;

   } else if (atoi(argv[2])==2) {
       cout << "Pool-scoped unordered_map using SlabAllocator\n";
       STLSlabAllocator<std::pair<string, int>> allocator = STLSlabAllocator<std::pair<string, int>>(64*1024*1024);
       hash<string> hasher = hash<string>();
       equal_to<string> key_equal=equal_to<string>();
       std::unordered_map<string, int, hash<string>, equal_to<string>,STLSlabAllocator<std::pair<string, int>>>* str_hash = new std::unordered_map<string, int, hash<string>, equal_to<string>,STLSlabAllocator<std::pair<string, int>>> (1140000, hasher, key_equal, allocator);
       if(!strcmp(argv[3], "sequentialstring")) {
           time_t before = time(0);
           for(i = 0; i< numKeysx10; i++) {
               for (j = 0; j < 10; j++ ) {
                  try{
                   str_hash->insert(std::pair<string, int>(new_string_from_integer(i*10+j), value));
                  }
                  catch(std::bad_alloc &e) {
                       cout <<"Caught the exception! Should add a new page, but simply exit now...\n";
                       exit(-1);
                  }
               }
           }
           time_t after = time(0);
           cout << "std unordered_map sequential string time:"<<after-before<<"\n";
       }
       else if(!strcmp(argv[3], "randomstring")){
           srandom(1);
           time_t before = time(0);
           for(i = 0; i< numKeysx10; i++) {
               for (j = 0; j < 10; j++ ) {
                   str_hash->insert(std::pair<string, int>(new_string_from_integer((int)random()), value));
               }
           }
           time_t after = time(0);
           cout << "std unordered_map random string time:"<<after-before<<"\n";
       }

   } else if (atoi(argv[2])==3) {
      cout << "Pangaea virtual hash map using ScopedAllocator\n";
      logger->writeLn("Pangaea virtual hash map using ScopedAllocator\n");
      ConfigurationPtr conf = make_shared<Configuration>();
      PDBLoggerPtr storageLogger = make_shared<PDBLogger>(conf->getLogFile());
      SharedMemPtr shm = make_shared<SharedMem>(conf->getShmSize(), storageLogger);
      PDBBufferPoolPtr networkBufferPool = make_shared<PDBBufferPool>(conf->getRecvBufferSize(), conf->getClientPageSize());
      PDBWorkerQueuePtr myWorkers = make_shared<PDBWorkerQueue>(storageLogger, conf->getMaxConnections());
      PDBStoragePtr storage = make_shared<PDBStorage>(shm, networkBufferPool, myWorkers, storageLogger, conf);
      storage->startFlushConsumerThreads();
      string path = "/data/pdbFiles";
      conf->createDir(path);
      SetID hashSetId;
      storage->addHashSet("testHashMap", hashSetId);
      HashSetPtr hashSet = storage->getHashSet(hashSetId);
      SetID tempSetId;
      storage->addTempSet("testHashMapSpill", tempSetId);
      TempSetPtr tempSet = storage->getTempSet(tempSetId);
      tempSet->pinBufferPage();
      TestVirtualHashMap * hashmap = new TestVirtualHashMap(220, 0, 0, 1040000, path, storage, hashSet, tempSet, nullptr, nullptr, nullptr);
       if(!strcmp(argv[3], "sequentialstring")) {
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 1000000; j++ ) {
                   hashmap->insert(new_string_from_integer(i*1000000+j), value);
               }
               cout << i <<":inserted 1000000 pairs\n";
           }
           time_t after = time(0);
           cout << "Pangaea hashmap sequential string insertion time:"<<after-before<<"\n";
           logger->writeLn("insertion time=");
           logger->writeInt(after-before);
       }
       else if(!strcmp(argv[3], "randomstring")){
           srandom(1);
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 1000000; j++ ) {
                   hashmap->insert(new_string_from_integer((int)random()), value);
               }
               cout << i<<":inserted 1000000 pairs\n";
           }
           time_t after = time(0);
           cout << "Pangaea hashmap random string time:"<<after-before<<"\n";
           logger->writeLn("time=");
           logger->writeInt(after-before);
      }
      storage->stopFlushConsumerThreads();
  }
   else if (atoi(argv[2])==4){
      cout << "Pangaea virtual hash map using SlabAllocator\n";
      logger->writeLn("Pangaea virtual hash map using SlabAllocator\n");
      ConfigurationPtr conf = make_shared<Configuration>();
      PDBLoggerPtr storageLogger = make_shared<PDBLogger>(conf->getLogFile());
      SharedMemPtr shm = make_shared<SharedMem>(conf->getShmSize(), storageLogger);
      PDBBufferPoolPtr networkBufferPool = make_shared<PDBBufferPool>(conf->getRecvBufferSize(), conf->getClientPageSize());
      PDBWorkerQueuePtr myWorkers = make_shared<PDBWorkerQueue>(storageLogger, conf->getMaxConnections());
      PDBStoragePtr storage = make_shared<PDBStorage>(shm, networkBufferPool, myWorkers, storageLogger, conf);
      storage->startFlushConsumerThreads();
      string path = "/data/pdbFilesSlab";
      conf->createDir(path);
      SetID hashSetId;
      storage->addHashSet("testHashMapSlab", hashSetId);
      HashSetPtr hashSet = storage->getHashSet(hashSetId);
      SetID tempSetId;
      storage->addTempSet("testHashMapSpill", tempSetId);
      TempSetPtr tempSet = storage->getTempSet(tempSetId);
      tempSet->pinBufferPage();
      TestVirtualHashMapWithSlabAllocator * hashmap = new TestVirtualHashMapWithSlabAllocator(220, 0, 0, 1100000, path, storage, hashSet, tempSet, nullptr, nullptr, nullptr);
       if(!strcmp(argv[3], "sequentialstring")) {
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 1000000; j++ ) {
                   hashmap->insert(new_string_from_integer(i*1000000+j), value);
               }
               cout <<i << ":inserted 1000000 pairs\n";
           }
           time_t after = time(0);
           cout << "Pangaea hashmap sequential string time:"<<after-before<<"\n";
           logger->writeLn("time=");
           logger->writeInt(after-before);
       }
       else if(!strcmp(argv[3], "randomstring")){
           srandom(1);
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 1000000; j++ ) {
                   hashmap->insert(new_string_from_integer((int)random()), value);
               }
               cout << i << ":inserted 1000000 pairs\n";
           }
           time_t after = time(0);
           cout << "Pangaea hashmap random string time:"<<after-before<<"\n";
           logger->writeLn("Pangaea insertion time=");
           logger->writeInt(after-before);
      }
      time_t beforeErase = time(0);
      int count = 0;
      char * curPointer;
      vector<PageIteratorPtr> * iterators = tempSet->getIterators();
      //iterate tempSet
      size_t size;
      int j=0;
      for (i = 0; i< iterators->size(); i++) {
          PageIteratorPtr iter = iterators->at(i);
          while(iter->hasNext()) {
              PDBPagePtr page = iter->next();
              if(page != nullptr) {
                 VariableSizeObjectIteratorPtr objectIter = make_shared<VariableSizeObjectIterator>(page);
                 while((curPointer = objectIter->nextRaw(size)) != nullptr) {
                    count ++;
                    if(count %1000000 == 0) {
                        cout << j<<": iterated 1000000 key-value pairs\n";
                        j++; 
                    }
                }
             }
             page->decRefCount();
          }

      }
      time_t afterErase = time(0);
      cout << "Pangaea hashmap iterating time:"<<afterErase-beforeErase<<"\n";
      logger->writeLn("Pangaea iterating time =");
      logger->writeInt(afterErase-beforeErase);
      storage->stopFlushConsumerThreads();
      cout << "count="<<count<<"\n";
  }
  else if (atoi(argv[2])==5) {
      /*
      cout << "Redis\n";
      redisContext * context = redisConnect("127.0.0.1", 6379);
      if(context == nullptr) {
          cout << "fail to create redis client...\n";
          exit (-1);
      }   
      void * reply = redisCommand(context, "FLUSHALL");
      freeReplyObject(reply);
      int pipelineSize = 64;
      int pipelineCounter = 0;
   
      if(!strcmp(argv[3], "sequentialstring")) {
          time_t before = time(0);
          for(i = 0; i< numKeysx1000000; i++) {
              for (j = 0; j < 1000000; j++ ) {
                 redisAppendCommand(context, "SET %s %d", new_string_from_integer(i*1000000+j).c_str(), value);
                 pipelineCounter ++;
                 if(pipelineCounter >= pipelineSize) {
                     void * reply;
                     int k = 0;
                     for (k=0; k<pipelineSize; k++) {
                         redisGetReply(context, &reply);
                         freeReplyObject((redisReply *) reply);
                     }
                     pipelineCounter = 0;
                 }
                 
              }
              cout <<i << ":inserted 1000000 pairs\n";
          }
          time_t after = time(0);
          cout << "Redis sequential string time:"<<after-before<<"\n";
          logger->writeLn("time=");
          logger->writeInt(after-before);
       }
       else if(!strcmp(argv[3], "randomstring")){
           srandom(1);
           time_t before = time(0);
           for(i = 0; i< numKeysx1000000; i++) {
               for (j = 0; j < 1000000; j++ ) {
                   redisAppendCommand(context, "SET %s %d", new_string_from_integer((int)random()).c_str(), value);
                   pipelineCounter ++;
                   if(pipelineCounter >= pipelineSize) {
                       void * reply;
                       int k = 0;
                       for (k=0; k<pipelineSize; k++) {
                           redisGetReply(context, &reply);
                           freeReplyObject((redisReply *) reply);
                       }
                       pipelineCounter = 0;
                   }
               }
               cout << i << ":inserted 1000000 pairs\n";
           }
           time_t after = time(0);
           cout << "Redis random string time:"<<after-before<<"\n";
           logger->writeLn("Redis insertion time=");
           logger->writeInt(after-before);
      }
      int count = 0;
      string cursor="";
      time_t beforeErase = time(0);
      reply = redisCommand(context, "SCAN 0");
      int i = 0;
      int j = 0;
      while (strcmp(cursor.c_str(), "0")!=0) {
          cursor = ((redisReply *)reply)->element[0]->str;
          if(strcmp(cursor.c_str(), "0")==0) {
              cout<<"returned all objects\n";
          }
          int numElements=((redisReply *) reply)->element[1]->elements;
          //cout << "num elements="<<numElements<<"\n";
          for (i = 0; i< numElements; i++) {
              //cout << ((redisReply *)reply)->element[1]->element[i]->str <<"\n";
              count ++;
              if(count%1000000==0) {
                  cout << j<<":iterated 1000000 objects\n";
                  j++;
              }
          }
          freeReplyObject(reply);
          if(strcmp(cursor.c_str(), "0") !=0) {
             reply = redisCommand(context, "SCAN %s", cursor.c_str());
          }
      }
      time_t afterErase = time(0);
      cout << "Redis iterating time:"<<afterErase-beforeErase<<"\n";
      logger->writeLn("Redis iteration time=");
      logger->writeInt(afterErase - beforeErase);
      cout << "count="<<count<<"\n";
      reply = redisCommand(context, "FLUSHALL");
      freeReplyObject(reply);     
    */ 
  }
}


