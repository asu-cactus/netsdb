
#ifndef TEST_DEDUPLICATION_CC
#define TEST_DEDUPLICATION_CC

#include "PDBString.h"
#include "PDBMap.h"
#include "TensorBlockIndex.h"
#include "InterfaceFunctions.h"
#include "PDBClient.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"
#include "FFMatrixBlockScanner.h"
#include "FFMatrixWriter.h"
#include "FFAggMatrix.h"
#include "FFTransposeMult.h"

#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <algorithm>
#include <random>
#include <algorithm>
#include <random>
using namespace std;
using namespace pdb;


void scan_data(pdb::PDBClient & pdbClient, std::string db_name, std::string set_name) {

  auto begin = std::chrono::high_resolution_clock::now();

  std::string errMsg;

  //create output set
  pdbClient.removeSet(db_name, "outputs", errMsg);
  pdbClient.createSet<FFMatrixBlock>(db_name, "outputs", errMsg,
                  DEFAULT_PAGE_SIZE, "outputs", nullptr, nullptr, false);


  // make the reader
  pdb::Handle<pdb::Computation> read =
      makeObject<FFMatrixBlockScanner>(db_name, set_name);

  // make the writer
  pdb::Handle<pdb::Computation> myWriter = nullptr;
  myWriter = pdb::makeObject<FFMatrixWriter>(db_name, "outputs");
  myWriter->setInput(read);

  bool materializeHash = false;

  auto exe_begin = std::chrono::high_resolution_clock::now();

  // run the computation
  if (!pdbClient.executeComputations(errMsg, set_name, materializeHash, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "****Scan End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  std::cout << "****Scan Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - exe_begin).count()
              << " secs." << std::endl;

  //verify the results
  int numBlocks = 0;
  auto it = pdbClient.getSetIterator<FFMatrixBlock>(db_name, "outputs");

  cout << "[STATS]: " << set_name << endl;
  for (auto r : it) {
      std::cout << r->getBlockRowIndex() << ":" << r->getBlockColIndex() << std::endl;
      numBlocks++;
  }
  std::cout << "Total:" << numBlocks << std::endl;

}

void load_data(pdb::PDBClient & pdbClient, std::string db_name, std::string set_name, std::vector<FFMatrixMeta> & listOfBlocks, int block_x, int block_y) {

     std::string errMsg;

     pdb::makeObjectAllocatorBlock(DEFAULT_PAGE_SIZE, true);

     pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> storeMatrix =
         pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

     int numBlocks = listOfBlocks.size();

     int numProcessedBlocks = 0;

     int i, j, x, y;

     double value;

     std::random_device rd;

     std::mt19937 e2(rd());

     std::uniform_real_distribution<> distp(0.0001, 0.5);

     std::uniform_real_distribution<> distn(-0.5, -0.0001);

     auto gen = std::bind(std::uniform_int_distribution<>(0, 1),
                       std::default_random_engine());

     while (numProcessedBlocks < numBlocks) {

          try {

              FFMatrixMeta meta = listOfBlocks[numProcessedBlocks];
              pdb::Handle<FFMatrixBlock> myData =
                  pdb::makeObject<FFMatrixBlock>(meta.blockRowIndex, meta.blockColIndex, block_x, block_y,
                                             meta.totalRows, meta.totalCols);
              for (int i = 0; i < block_x; i++) {
                  for (int j = 0; j < block_y; j++) {
                       value = (bool) gen()   ? distn(e2) : distp(e2);
                       (*(myData->getRawDataHandle()))[i * block_y + j] = value;
                  }
              }

              storeMatrix->push_back(myData);
              numProcessedBlocks++;
          } catch (pdb::NotEnoughSpace &e) {

              if (!pdbClient.sendData<FFMatrixBlock> (pair<string, string>(set_name, db_name), storeMatrix, errMsg)) {
                  cout << "Failed to send data to dispatcher server" << endl;
                  exit(1);
              }
              cout << "Dispatched " << storeMatrix->size() << " blocks." << std::endl;
              pdb::makeObjectAllocatorBlock(DEFAULT_PAGE_SIZE, true);
              storeMatrix = pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();
          }

     }

     pdbClient.flushData(errMsg);

}

void load_shared_set(pdb::PDBClient & pdbClient, std::string db_name, std::string set_name, std::vector<FFMatrixMeta> & listOfSharedBlocks, int block_x, int block_y) {

     std::string errMsg;
     pdbClient.removeSet(db_name, set_name, errMsg);
     pdbClient.createSet<FFMatrixBlock>(db_name, set_name, errMsg,
                     DEFAULT_PAGE_SIZE, "sharedSet", nullptr, nullptr, true);
     load_data(pdbClient, db_name, set_name, listOfSharedBlocks, block_x, block_y);


}

void load_set(pdb::PDBClient & pdbClient, std::string db_name, std::string set_name, std::vector<FFMatrixMeta> & listOfPrivateBlocks, std::vector<std::pair<PageID, PageIndex>> & pageSharing, int block_x, int block_y, std::string shared_db_name, std::string shared_set_name) {

     std::string errMsg;
     pdbClient.removeSet(db_name, set_name, errMsg);
     //create a normal set 
     pdbClient.createSet<FFMatrixBlock>(db_name, set_name, errMsg,
                     DEFAULT_PAGE_SIZE, set_name, nullptr, nullptr, false);

     //load private pages to the normal set
     load_data(pdbClient, db_name, set_name, listOfPrivateBlocks, block_x, block_y);


     sleep(60);

     //load pointers that points to the shared pages
     for (int i = 0; i < pageSharing.size(); i++) {
         bool whetherToCreateSharedSet = false;
	 if (i==0)
              whetherToCreateSharedSet = true;
         else
	      whetherToCreateSharedSet = false;	 
	 std::pair<PageID, PageIndex> sharing = pageSharing[i];
	 pdbClient.addSharedPage(db_name, set_name, "FFMatrixBlock",
                    shared_db_name, shared_set_name, "FFMatrixBlock",
                   sharing.first, sharing.second.partitionId, sharing.second.pageSeqInPartition, whetherToCreateSharedSet, 0, errMsg);
         
     }
}

int main(int argc, char* argv[]) {

    //This test case simulates the deduplication process of two heterogeneous tensors.	
    //We use the block size of 100x10000
    //The first heterogeneous tensor has a dimension of 500x1000000: 5x100 blocks
    //The second heterogeneous tensor has a dimension of 600x900000: 6x90 blocks
    //In the shared set, there are 450 shared blocks are:
    //T1(0, 0) -> T2(5, 89)
    //T1(0, 1) -> T2(5, 88)
    //T1(0, 2) -> T2(5, 87)
    //...
    //T1(0, 89)-> T2(5, 0)
    //T1(1, 0) -> T2(4, 89)
    //T1(1, 1) -> T2(4, 88)
    //T1(1, 2) -> T2(4, 87)
    //...
    //T1(1, 89)-> T2(4, 0)
    //T1(2, 0) -> T2(3, 89)
    //T1(2, 1) -> T2(3, 88)
    //T1(2, 2) -> T2(3, 87)
    //...
    //T1(2, 89)-> T2(3, 0)
    //T1(3, 0) -> T2(2, 89)
    //T1(3, 1) -> T2(2, 88)
    //T1(3, 2) -> T2(2, 87)
    //...
    //T1(3, 89)-> T2(2, 0)
    //T1(4, 0) -> T2(1, 89)
    //T1(4, 1) -> T2(1, 88)
    //T1(4, 2) -> T2(1, 87)
    //...
    //T1(4, 89)-> T2(1, 0)
    //These shared blocks are packed to 14 pages, with each page having 256MB
    //In the set for T1, there are 50 blocks
    //T1(0, 90) ... T1(0, 99)
    //T1(1, 90) ... T1(1, 99)
    //T1(2, 90) ... T1(2, 99)
    //T1(3, 90) ... T1(3, 99)
    //T1(4, 90) ... T1(4, 99)
    //In the set for T2, there are 40 blocks
    //T2 (0, 0) ... T2(0, 89)
    

    //create a shared set
     string masterIp = "localhost";
     pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("TestSharedSetLog");
     pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
     pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);



     std::string errMsg;
     int block_x = 100;
     int block_y = 10000;

     std::string databaseName = "deduplication_test";
     std::string sharedSetName = "shared";
     std::string tensor1SetName = "t1";
     std::string tensor2SetName = "t2";

     pdbClient.createDatabase(databaseName, errMsg);
     makeObjectAllocatorBlock(128*1024*1024, true);
     pdbClient.registerType("libraries/libFFMatrixMeta.so", errMsg);
     pdbClient.registerType("libraries/libFFMatrixData.so", errMsg);
     pdbClient.registerType("libraries/libFFMatrixBlock.so", errMsg);
     pdbClient.registerType("libraries/libFFMatrixBlockScanner.so", errMsg);
     pdbClient.registerType("libraries/libFFMatrixWriter.so", errMsg);

     //Create a shared set.

     std::cout << "############To create the shared set ...###############" << std::endl; 

     //A list of metadata for shared blocks. An alternative approach is to load from a file
     std::vector<FFMatrixMeta> listOfSharedBlocks;
     for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 90; j++) {
            FFMatrixMeta meta(i, j, i, j);//for shared blocks, we use the totalRows field to store the static block row Id, and use the totalCols field to store the static block col id
            listOfSharedBlocks.push_back(meta);	    
	}
     }

     //load the shared set
     load_shared_set(pdbClient, databaseName, sharedSetName, listOfSharedBlocks, block_x, block_y);
	

     std::cout << "############To create the set for T1 ...###############" << std::endl;



     //A list of metadata for T1 blocks. An alternative approach is to load from a file
     std::vector<FFMatrixMeta> listOfT1Blocks;
     for (int i = 0; i < 5; i++) {
        for (int j = 90; j < 100; j++) {
            FFMatrixMeta meta(i, j, 500, 1000000);
            listOfT1Blocks.push_back(meta);
        }
     }

     //A list of pages in the shared set (We need load_shared_set first to obtain the pageIndex)
     std::vector<std::pair<PageID, PageIndex>>  pageSharingInT1;
     for (int i = 0; i < 13; i++) {
         PageIndex index;
	 index.partitionId = 0;
	 index.pageSeqInPartition = i;
    	 pageSharingInT1.push_back(std::pair<PageID, PageIndex>(i, index));
     }
     

     //load T1
     load_set(pdbClient, databaseName, tensor1SetName, listOfT1Blocks, pageSharingInT1, block_x, block_y, databaseName, sharedSetName);


     std::cout << "############To create the set for T2 ...###############" << std::endl;

     //A list of metadata for T2 blocks. An alternative approach is to load from a file
     std::vector<FFMatrixMeta> listOfT2Blocks;
     for (int i = 0; i < 90; i++) {
         FFMatrixMeta meta(0, i, 600, 900000);
	 listOfT2Blocks.push_back(meta);
     }  

     //A list of pages in the shared set (We need load_shared_set first to obtain the pageIndex)
     //
     //
     std::vector<std::pair<PageID, PageIndex>>  pageSharingInT2;
     for (int i = 0; i < 13; i++) {
         PageIndex index;
         index.partitionId = 0;
         index.pageSeqInPartition = i;
         pageSharingInT2.push_back(std::pair<PageID, PageIndex>(i, index));
     }
     //load T2
     load_set(pdbClient, databaseName, tensor2SetName, listOfT2Blocks, pageSharingInT2, block_x, block_y, databaseName, sharedSetName);  


      std::cout << "############To add block mappings ...###############" << std::endl;

     //create the block mapping
     pdbClient.addSharedMapping(databaseName, tensor2SetName, "FFMatrixBlock", databaseName, sharedSetName, "FFMatrixBlock", "/home/ubuntu/mapping.csv",600, 900000, errMsg);

     //scan datasets
     scan_data (pdbClient, databaseName, tensor1SetName);

     scan_data (pdbClient, databaseName, tensor2SetName);    


     return 0;


     

}

#endif
