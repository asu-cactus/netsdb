
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


void load_data(pdb::PDBClient & pdbClient, std::string db_name, std::string set_name, std::vector<FFMatrixMeta> & listOfBlocks, int block_x, int block_y) {

     std::string errMsg;

     pdb::makeObjectAllocatorBlock(DEFAULT_PAGE_SIZE * 1024 * 1024, true);

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
              pdb::makeObjectAllocatorBlock(DEFAULT_PAGE_SIZE * 1024 * 1024, true);
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

     //load pointers that points to the shared pages
     for (int i = 0; i < pageSharing.size(); i++) {
     
	 std::pair<PageID, PageIndex> sharing;
	 pdbClient.addSharedPage(db_name, set_name, "FFMatrixBlock",
                    shared_db_name, shared_set_name, "FFMatrixBlock",
                   sharing.first, sharing.second.partitionId, sharing.second.pageSeqInPartition, true, 0, errMsg);
     
     }
}

int main(int argc, char* argv[]) {

    return 0;

}

#endif
