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

#include <fstream>      // fstream
#include <boost/tokenizer.hpp>
#include <cstddef>
#include <iostream>
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
#include <random>

using namespace std;
using namespace boost;

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

void load_independent_FF_sets(pdb::PDBClient pdbClient, std::string databaseName, int block_x, int block_y, int batch_size, int numFeatures, int numNeurons, int numLabels) {

      std::string errMsg;

      ff::createDatabase(pdbClient, databaseName);
      ff::setup(pdbClient, databaseName);

      ff::createSet(pdbClient, databaseName, "inputs", "inputs", 64);
      ff::createSet(pdbClient, databaseName, "label", "label", 64);

      //ff::createSet(pdbClient, databaseName, "w1", "W1", 64);
      ff::createSet(pdbClient, databaseName, "b1", "B1", 64);

      ff::createSet(pdbClient, databaseName, "wo", "WO", 64);
      ff::createSet(pdbClient, databaseName, "bo", "BO", 64);

      std::cout << "To load matrix for "<< databaseName <<":inputs" << std::endl;
      ff::loadMatrix(pdbClient, databaseName, "inputs", batch_size, numFeatures, block_x,
                   block_y, false, false, errMsg);

      //std::cout << "To load matrix for "<< databaseName <<" w1" << std::endl;
      //ff::loadMatrix(pdbClient, databaseName, "w1", numNeurons, numFeatures, block_x, block_y,
        //           false, false, errMsg);

      std::cout << "To load matrix for " << databaseName << ":b1" << std::endl;
      ff::loadMatrix(pdbClient, databaseName, "b1", numNeurons, 1, block_x,
                   block_y, false, true, errMsg);

      std::cout << "To load matrix for " << databaseName << ":wo" << std::endl;
      ff::loadMatrix(pdbClient, databaseName, "wo", numLabels, numNeurons, block_x,
                   block_y, false, false, errMsg);

      std::cout << "To load matrix for " << databaseName << ":bo" << std::endl;
      ff::loadMatrix(pdbClient, databaseName, "bo", numLabels, 1, block_x, block_y,
                   false, true, errMsg);

}


void load_output_sets(pdb::PDBClient pdbClient, std::string databaseName) {

     ff::createSet(pdbClient, databaseName, "output", "Output", 256);
     ff::createSet(pdbClient, databaseName, "y1", "Y1", 256);
     ff::createSet(pdbClient, databaseName, "yo", "YO", 256);

}

void load_private_set(pdb::PDBClient pdbClient, std::string databaseName, std::string private_blocks_path) {


     //A list of metadata for T1 blocks. An alternative approach is to load from a file
     std::vector<FFMatrixMeta> listOfT1Blocks;
     //TODO


     //A list of pages in the shared set (We need load_shared_set first to obtain the pageIndex)
     std::vector<std::pair<PageID, PageIndex>>  pageSharingInT1;
     for (int i = 0; i < 13; i++) {
         PageIndex index;
         index.partitionId = 0;
         index.pageSeqInPartition = i;
         pageSharingInT1.push_back(std::pair<PageID, PageIndex>(i, index));
     }


     //load T1
     load_set(pdbClient, databaseName, "w1", listOfT1Blocks, pageSharingInT1, 50, 10000, "shared_db", "shared_set");

}

void runFF(pdb::PDBClient pdbClient, std::string databaseName) {

  double dropout_rate = 0.5;

  auto begin = std::chrono::high_resolution_clock::now();

  ff::inference_unit(pdbClient, databaseName, "w1", "wo", "inputs", "b1", "bo",
                "output", dropout_rate);

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "*****FFTest End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

}


int main(int argc, char *argv[]) {

  bool reloadData = true;

  if (argc < 2) {
  
	  std::cout << "Usage: shared_block_path  whetherToLoadData(Y/N)" << std::endl;
  
  }

  string errMsg;
  
  std::string shared_block_path = argv[1];

  if (argc >= 2) {
      if (strcmp(argv[2], "N")==0) {
          reloadData = false;
          std::cout << "WARNING: we do not reload data!" << std::endl;
      } else {
          std::cout << "WARNING: we reload data!" << std::endl;
      }
  }

  bool generate = true;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("FFclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);


  if (reloadData) {

	  //create other sets for amazoncat-13k
          load_independent_FF_sets(pdbClient, "amazoncat-13k", 50, 10000, 1000, 203882, 1000, 1330);
	  

	  //create other sets for amazoncat-14k
          load_independent_FF_sets(pdbClient, "amazoncat-14k", 50, 10000, 1000, 597540, 1000, 14588);

	  //create other sets for EURlex-4.3k
          load_independent_FF_sets(pdbClient, "EURlex-4.3k", 50, 10000, 1000, 200000, 2000, 4271);


	  //create other sets for w0 in RCV1-2k
          load_independent_FF_sets(pdbClient, "RCV1-2k", 50, 10000, 1000, 47236, 5000, 2456);

	  //create the shared set
          std::vector<FFMatrixMeta> listOfSharedBlocks;
          ifstream in(shared_block_path.c_str());
          if (!in.is_open()) {
              std::cout << "FATAL ERROR: Cannot open file: " << shared_block_path << std::endl;
              return -1;
	  }
          typedef boost::tokenizer< escaped_list_separator<char> > Tokenizer;
          std::string line;
          std::vector< std::string > vec;
          while (getline(in,line)) {
             Tokenizer tok(line);
             vec.assign(tok.begin(),tok.end());
             Handle<FFMatrixMeta> sharedBlockMeta = makeObject<FFMatrixMeta>(atoi(vec[1].c_str()), atoi(vec[2].c_str()), atoi(vec[0].c_str()), 0);
	     listOfSharedBlocks.push_back(*sharedBlockMeta);
          }

	  //load the shared set
          load_shared_set(pdbClient, "shared_db", "shared_set", listOfSharedBlocks, 50, 10000);
    
	  //create the private set for w0 in amazoncat-13k
	  
	  //create the private set for w0 in amazoncat-14k
	  
	  //create the private set for w0 in EURlex-4.3k
	  
	  //create the private set for w0 in RCV1-2k
	  
	  //add the block mapping to shared set
	  

	  
	  

  }

  //execute the models
  load_output_sets(pdbClient, "amazoncat-13k");
  runFF(pdbClient, "amazoncat-13k");

  load_output_sets(pdbClient, "amazoncat-14k");
  runFF(pdbClient, "amazoncat-14k");

  load_output_sets(pdbClient, "EURlex-4.3k");
  runFF(pdbClient, "EURlex-4.3k");

  load_output_sets(pdbClient, "RCV1-2k");
  runFF(pdbClient, "RCV1-2k");

  return 0;
}
