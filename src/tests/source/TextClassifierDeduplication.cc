#include "PDBString.h"
#include "FFAggMatrixToOneMatrix.h"
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
#include "FFInputLayerJoin.h"
#include "SemanticClassifierSingleBlock.h"

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
#include <stdlib.h> 

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
                                             meta.totalRows, meta.totalCols, meta.distinctBlockId);
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
     if (!pdbClient.sendData<FFMatrixBlock> (pair<string, string>(set_name, db_name), storeMatrix, errMsg)) {
                  cout << "Failed to send data to dispatcher server" << endl;
                  exit(1);
     }
     cout << "Dispatched " << storeMatrix->size() << " blocks." << std::endl;
     pdbClient.flushData(errMsg);

}


void load_shared_set(pdb::PDBClient & pdbClient, std::string db_name, std::string set_name, std::vector<FFMatrixMeta> & listOfSharedBlocks, int block_x, int block_y, bool append) {

     std::string errMsg;
     if (append == false) {
         ff::createDatabase(pdbClient, db_name);
         pdbClient.removeSet(db_name, set_name, errMsg);
         pdbClient.createSet<FFMatrixBlock>(db_name, set_name, errMsg,
                     DEFAULT_PAGE_SIZE, "sharedSet", nullptr, nullptr, true);
     }
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

void load_independent_input_sets(pdb::PDBClient pdbClient, std::string databaseName, int block_x, int block_y, int batch_size, int numFeatures) {

      std::string errMsg;

      ff::createDatabase(pdbClient, databaseName);

      ff::createSet(pdbClient, databaseName, "inputs", "inputs", 64);

      std::cout << "To load matrix for "<< databaseName <<":inputs" << std::endl;
      ff::loadMatrix(pdbClient, databaseName, "inputs", batch_size, numFeatures, block_x,
                   block_y, false, false, errMsg);

}


void load_output_sets(pdb::PDBClient pdbClient, std::string databaseName) {


     ff::createSet(pdbClient, databaseName, "outputs", "Outputs", 256);
     ff::createSet(pdbClient, databaseName, "intermediate", "Intermediate", 256);
}

void load_private_set(pdb::PDBClient pdbClient, std::string databaseName, std::string private_blocks_path, std::map<int, std::pair<int, int>> & distinctBlockMap, std::vector<int> pageIds, int totalX, int totalY) {


     //A list of metadata for T1 blocks. An alternative approach is to load from a file
     std::vector<FFMatrixMeta> listOfPrivateBlocks;
     //create the list of private blocks
     ifstream in(private_blocks_path.c_str());
     if (!in.is_open()) {
         std::cout << "FATAL ERROR: Cannot open file: " << private_blocks_path << std::endl;
         return;
     }
     typedef boost::tokenizer< escaped_list_separator<char> > Tokenizer;
     std::string line;
     while (getline(in,line)) {
         std::cout << line << std::endl;
	 int blockId = atoi(line.c_str());
	 std::pair<int, int> block_indexes = distinctBlockMap[blockId];
         Handle<FFMatrixMeta> privateBlockMeta = makeObject<FFMatrixMeta>(block_indexes.first, block_indexes.second, totalX, totalY);
         listOfPrivateBlocks.push_back(*privateBlockMeta);
     }
     in.close();

     //A list of pages in the shared set (We need load_shared_set first to obtain the pageIndex)
     std::vector<std::pair<PageID, PageIndex>>  pageSharing;
     for (int i = 0; i < pageIds.size(); i++) {
         PageIndex index;
         index.partitionId = 0;
         index.pageSeqInPartition = pageIds[i];
         pageSharing.push_back(std::pair<PageID, PageIndex>(pageIds[i], index));
     }

     //load T1
     load_set(pdbClient, databaseName, "embedding", listOfPrivateBlocks, pageSharing, 50, 10000, "shared_db", "shared_set");

}

void runWorkload(pdb::PDBClient pdbClient, std::string databaseName, int embedding_dimension) {

    std::cout << "TO RUN WORKLOAD ON " << databaseName << " with an embedding dimension of " << embedding_dimension << std::endl;

    pdb::makeObjectAllocatorBlock(128*1024*1024, true);

    std::string errMsg;

    auto begin = std::chrono::high_resolution_clock::now();

    // make the reader
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>(databaseName, "embedding");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>(databaseName, "inputs");

    // make the transpose multiply join
    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the transpose multiply aggregation
    pdb::Handle<pdb::Computation> myAggregation =
        pdb::makeObject<FFAggMatrix>();

    myAggregation->setOutput(databaseName, "intermediate");
    myAggregation->setInput(join);

    pdb::Handle<pdb::Computation> readC =
        makeObject<FFMatrixBlockScanner>(databaseName, "intermediate");

    // merge the all FFMatrixcBlocks to one single FFMatrix
    pdb::Handle<pdb::Computation> myAggregation1 =
        pdb::makeObject<FFAggMatrixToOneMatrix>();
    myAggregation1->setInput(readC);

    // make the classifier
    uint32_t sizeDense0 = 16;
    uint32_t sizeDense1 = 1;



    // SemanticClassifierSingleBlock takes the input as FFSingleMatrix
    pdb::Handle<pdb::Computation> classifier =
        pdb::makeObject<SemanticClassifierSingleBlock>(embedding_dimension, sizeDense0, sizeDense1);
    classifier->setInput(myAggregation1);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = nullptr;
    myWriter =
        pdb::makeObject<FFMatrixWriter>(databaseName, "outputs");
    myWriter->setInput(classifier);

    bool materializeHash = false;

    auto exe_begin = std::chrono::high_resolution_clock::now();

    // run the computation
    if (!pdbClient.executeComputations(errMsg, databaseName, materializeHash,
                                   myAggregation)) {
        cout << "Computation failed. Message was: " << errMsg << "\n";
        exit(1);
    }

        // run the computation
    if (!pdbClient.executeComputations(errMsg, "aggregation-1", materializeHash,
                                   myWriter)) {
        cout << "Computation failed. Message was: " << errMsg << "\n";
        exit(1);
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "****Text Classification End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end -
                                                                          begin)
                     .count()
              << " secs." << std::endl;

    std::cout << "****Text Classification Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(
                     end - exe_begin)
                     .count()
              << " secs." << std::endl;

    // verify the results
    ff::print_stats(pdbClient, databaseName, "outputs");
    ff::print(pdbClient, databaseName, "outputs");
    ff::print_stats(pdbClient, databaseName, "intermediate");
    ff::print(pdbClient, databaseName, "intermediate");
}


void createAndLoadSharedSet(pdb::PDBClient pdbClient, std::map<int, std::pair<int, int>> & distinctBlockMap, std::string shared_block_path) {

   //create the shared set
   std::vector<std::vector<FFMatrixMeta>> listsOfSharedBlocks;
   ifstream shared_in(shared_block_path.c_str());
   if (!shared_in.is_open()) {
       std::cout << "FATAL ERROR: Cannot open file: " << shared_block_path << std::endl;
       return;
   }
   std::vector<FFMatrixMeta> curVec;
   std::string line;
   while (getline(shared_in,line)) {
       std::cout << line << std::endl;
       if (strncmp(line.c_str(), "N", 1)==0){
	       std::cout <<"#: to push back the curVec" << std::endl;
	       listsOfSharedBlocks.push_back(curVec);
	       curVec.clear();
       } else {
           int blockId = atoi(line.c_str());
           if (distinctBlockMap.count(blockId) > 0) {
                std::pair<int, int> block_indexes = distinctBlockMap[blockId];
                std::cout << "-------to push back a new FFMatrixMeta" << block_indexes.first << ", " << block_indexes.second << std::endl;
	        Handle<FFMatrixMeta> sharedBlockMeta = makeObject<FFMatrixMeta>(block_indexes.first, block_indexes.second, 0, 0);
                sharedBlockMeta->distinctBlockId = blockId;
		curVec.push_back(*sharedBlockMeta);
           }
       }
   }
   std::cout << "We found " << listsOfSharedBlocks.size() << " EQUIVALENT CLASSES" << std::endl;
   bool append = false;
   for (int i = 0; i < listsOfSharedBlocks.size(); i++) {
	std::cout << "\r\nEQUIVALENT CLASS-" << i << ":";
        for (int j = 0; j < listsOfSharedBlocks[i].size(); j++) {
            std::cout << listsOfSharedBlocks[i][j].blockRowIndex << ":" << listsOfSharedBlocks[i][j].blockColIndex << ":"<< listsOfSharedBlocks[i][j].distinctBlockId << ";";	
	}
	load_shared_set(pdbClient, "shared_db", "shared_set", listsOfSharedBlocks[i], 50, 10000, append);
	if (append == false) {
	    append = true;
	}
   }

   shared_in.close();
}

int main(int argc, char *argv[]) {

  bool reloadData = true;

  if (argc < 3) {
  
	  std::cout << "Usage: distinct_block_path whetherToLoadData(Y/N) whichModelToRun (0 for not running any model, 1 for running model-1(nnlm-128), 2 for running model-2(nnlm-50), 3 for running model-3(wiki-250), 4 for running model-4(wiki-500))" << std::endl;
  
  }

  string errMsg;
  
  std::string distinct_block_path = argv[1];
  if (argc >= 2) {
      if (strcmp(argv[2], "N")==0) {
          reloadData = false;
          std::cout << "WARNING: we do not reload data!" << std::endl;
      } else {
          std::cout << "WARNING: we reload data!" << std::endl;
      }
  }

  int whichModelToRun = 0;
  if (argc >= 3) {
      whichModelToRun = atoi(argv[3]);
  }

  bool generate = true;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("FFclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);
 


  pdb::makeObjectAllocatorBlock(128*1024*1024, true);

  if (reloadData) {


          ff::loadLibrary(pdbClient, "libraries/libFFInputLayerJoin.so");
          ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
          ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
          ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
          ff::loadLibrary(pdbClient, "libraries/libFFSingleMatrix.so");
          ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
          ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
          ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
          ff::loadLibrary(pdbClient, "libraries/libFFAggMatrixToOneMatrix.so");
          ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
          ff::loadLibrary(pdbClient, "libraries/libSemanticClassifierSingleBlock.so");

	  //create the list of distinct blocks
          ifstream distinct_in(distinct_block_path.c_str());
          if (!distinct_in.is_open()) {
              std::cout << "FATAL ERROR: Cannot open file: " << distinct_block_path << std::endl;
              return -1;
          }
          typedef boost::tokenizer< escaped_list_separator<char> > Tokenizer;
          std::string line;
          std::vector< std::string > vec;
	  std::map<int, std::pair<int, int>> distinctBlockMap;
          while (getline(distinct_in,line)) {
             std::cout << line << std::endl;
             Tokenizer tok(line);
             vec.assign(tok.begin(),tok.end());
	     distinctBlockMap[atoi(vec[0].c_str())] = std::pair<int, int>(atoi(vec[2].c_str()), atoi(vec[1].c_str()));
             std::cout << atoi(vec[0].c_str()) << " => " << "(" << atoi(vec[2].c_str()) << ", " << atoi(vec[1].c_str()) << ")";
	  }
      

	  distinct_in.close();

          //create and load shared set
          createAndLoadSharedSet(pdbClient, distinctBlockMap, "/home/ubuntu/shared-1-3/file2/exp1/file2-shared.csv");


          //load independent input sets
          load_independent_input_sets(pdbClient, "nnlm_128", 50, 10000, 100, 970000);
	  load_independent_input_sets(pdbClient, "nnlm_50", 50, 10000, 100, 970000);
          load_independent_input_sets(pdbClient, "wiki_250", 50, 10000, 100, 1010000);
	  load_independent_input_sets(pdbClient, "wiki_500", 50, 10000, 100, 1010000);

          std::vector<int> pageIds1;
          pageIds1.push_back(0);
	  pageIds1.push_back(1);
	  pageIds1.push_back(2);
	  pageIds1.push_back(3);
	  pageIds1.push_back(4);
	  pageIds1.push_back(5);
	  pageIds1.push_back(7);
	  pageIds1.push_back(8);
          pageIds1.push_back(9);

	  //create the private set for the embedding layer nnlm_dim_128_yelp
          load_private_set(pdbClient, "nnlm_128", "/home/ubuntu/shared-1-3/file2/exp1/file2_0.csv", distinctBlockMap, pageIds1, 150, 970000);

          std::vector<int> pageIds2;
	  pageIds2.push_back(0);
	  pageIds2.push_back(1);
          pageIds2.push_back(2);
	  pageIds2.push_back(3);
	  pageIds2.push_back(6);
	  pageIds2.push_back(7);
	  
	  //create the private set for the embedding layer nnlm_dim_50_imdb
	  load_private_set(pdbClient, "nnlm_50", "/home/ubuntu/shared-1-3/file2/exp1/file2_5.csv", distinctBlockMap, pageIds2, 50, 970000);

          std::vector<int> pageIds3;
          pageIds3.push_back(0);
          pageIds3.push_back(1);
	  pageIds3.push_back(2);
	  pageIds3.push_back(3);
	  pageIds3.push_back(4);
          pageIds3.push_back(5);
	  pageIds3.push_back(6);
	  pageIds3.push_back(8);
          pageIds3.push_back(10);

	  //create the private set for the embedding layer wiki_250_civil
          load_private_set(pdbClient, "wiki_250", "/home/ubuntu/shared-1-3/file2/exp1/file2_10.csv", distinctBlockMap, pageIds3, 250, 1010000);

	  std::vector<int> pageIds4;
          pageIds4.push_back(0);
	  pageIds4.push_back(1);
          pageIds4.push_back(2);
	  pageIds4.push_back(4);
	  pageIds4.push_back(5);
	  pageIds4.push_back(6);
	  pageIds4.push_back(9);
          pageIds4.push_back(10);

	  //create the private set for the embedding layer wiki_500_yelp
	  load_private_set(pdbClient, "wiki_500", "/home/ubuntu/shared-1-3/file2/exp1/file2_15.csv", distinctBlockMap, pageIds4, 500, 1010000);
	  
	  
	  
	  //add the block mapping to shared set
	  pdbClient.addSharedMapping("nnlm_128", "embedding", "FFMatrixBlock", "shared_db", "shared_set", "FFMatrixBlock", "/home/ubuntu/shared-1-3/file1-3/exp1/file3-tensor0.csv", 150, 970000, true, errMsg);
          pdbClient.addSharedMapping("nnlm_50", "embedding", "FFMatrixBlock", "shared_db", "shared_set", "FFMatrixBlock", "/home/ubuntu/shared-1-3/file1-3/exp1/file3-tensor5.csv", 50, 970000, true, errMsg);
          pdbClient.addSharedMapping("wiki_250", "embedding", "FFMatrixBlock", "shared_db", "shared_set", "FFMatrixBlock", "/home/ubuntu/shared-1-3/file1-3/exp1/file3-tensor10.csv", 250, 1010000, true, errMsg);
	  pdbClient.addSharedMapping("wiki_500", "embedding", "FFMatrixBlock", "shared_db", "shared_set", "FFMatrixBlock", "/home/ubuntu/shared-1-3/file1-3/exp1/file3-tensor15.csv", 500, 1010000, true, errMsg);
	  

  }

  //execute the models
  if (whichModelToRun == 1) {

      load_output_sets(pdbClient, "nnlm_128");
      runWorkload(pdbClient, "nnlm_128", 128);
  } else if (whichModelToRun == 2) {

      load_output_sets(pdbClient, "nnlm_50");
      runWorkload(pdbClient, "nnlm_50", 50);

  } else if (whichModelToRun == 3) {

      load_output_sets(pdbClient, "wiki_250");
      runWorkload(pdbClient, "wiki_250", 250);

  } else if (whichModelToRun == 4) {

      load_output_sets(pdbClient, "wiki_500");
      runWorkload(pdbClient, "wiki_500", 500);

  } else {
  
      std::cout << "We will not run any model this time" << std::endl;
  }

  return 0;
}
