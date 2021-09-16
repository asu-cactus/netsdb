
#ifndef TEST_WORD2VEC_WITH_DEDUPLICATION_CC
#define TEST_WORD2VEC_WITH_DEDUPLICATION_CC

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

using namespace pdb;

void execute(pdb::PDBClient & pdbClient, std::string weight_set_name) {

  auto begin = std::chrono::high_resolution_clock::now();

  // make the reader
  pdb::Handle<pdb::Computation> readA =
      makeObject<FFMatrixBlockScanner>("word2vec", weight_set_name);
  pdb::Handle<pdb::Computation> readB =
      makeObject<FFMatrixBlockScanner>("word2vec", "inputs");

  // make the transpose multiply join
  pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
  join->setInput(0, readA);
  join->setInput(1, readB);

  // make the transpose multiply aggregation
  pdb::Handle<pdb::Computation> myAggregation =
      pdb::makeObject<FFAggMatrix>();
  myAggregation->setInput(join);

  // make the writer
  pdb::Handle<pdb::Computation> myWriter = nullptr;
  myWriter = pdb::makeObject<FFMatrixWriter>("word2vec", "outputs");
  myWriter->setInput(myAggregation);

  bool materializeHash = false;
  std::string errMsg;

  auto exe_begin = std::chrono::high_resolution_clock::now();

  // run the computation
  if (!pdbClient.executeComputations(errMsg, weight_set_name, materializeHash, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "****Word2Vec End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  std::cout << "****Word2Vec Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - exe_begin).count()
              << " secs." << std::endl;

  //verify the results
  ff::print_stats(pdbClient, "word2vec", "outputs");
  ff::print(pdbClient, "word2vec", "outputs");

}


int main(int argc, char* argv[]) {

     bool loadData = true;
     if (argc > 1) {
        if (strcmp(argv[1], "N") == 0) {
           loadData = false;
	}
     }
     makeObjectAllocatorBlock(124 * 1024 * 1024, true);

    //create a shared set
     string masterIp = "localhost";
     pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("TestSharedSetLog");
     pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
     pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

     ff::createDatabase(pdbClient, "word2vec");
     ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
     ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
     ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
     ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
     ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
     ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
     ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
     std::string errMsg;
     pdbClient.createSet<FFMatrixBlock>("word2vec", "shared_weights", errMsg,
		     DEFAULT_PAGE_SIZE, "weights", nullptr, nullptr, true);

     int block_x = 100;
     int block_y = 10000;
     int matrix1_totalNumBlock_y = 100;
     int matrix2_totalNumBlock_y = 100;
     int sharedNumBlock_y = 90;
     int numBlock_x = 5;

     int batchSize = 100;

     
     if (loadData) {
     //load blocks to the shared set
     ff::loadMatrix(pdbClient, "word2vec", "shared_weights", numBlock_x*block_x, sharedNumBlock_y*block_y, block_x, block_y, false, false, errMsg);
      
     //create private set 1
     pdbClient.createSet<FFMatrixBlock>("word2vec", "weights1", errMsg,
                     DEFAULT_PAGE_SIZE, "weights1", nullptr, nullptr, false);
    
     //load blocks to the private set 1
     ff::loadMatrix(pdbClient, "word2vec", "weights1", numBlock_x*block_x, (matrix1_totalNumBlock_y-sharedNumBlock_y)*block_y, block_x, block_y, false, false, errMsg);

     sleep(30);

     //add the metadata of shared pages to the private set 1
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
		    "word2vec", "shared_weights", "FFMatrixBlock",
		   0, 0, 0, true, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   1, 0, 1, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   2, 0, 2, false, 0, errMsg);  
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   3, 0, 3, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   4, 0, 4, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   5, 0, 5, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   6, 0, 6, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   7, 0, 7, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   8, 0, 8, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   9, 0, 9, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   10, 0, 10, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   11, 0, 11, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   12, 0, 12, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights1", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   13, 0, 13, false, 0, errMsg);

     //create private set 2
     pdbClient.createSet<FFMatrixBlock>("word2vec", "weights2", errMsg,
                     DEFAULT_PAGE_SIZE, "weights1", nullptr, nullptr, false);
    
     //load blocks to the private set 2
     ff::loadMatrix(pdbClient, "word2vec", "weights2", numBlock_x*block_x, (matrix2_totalNumBlock_y-sharedNumBlock_y)*block_y, block_x, block_y, false, false, errMsg);
     sleep(30);

     //add the metadata of shared pages to the private set 2
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   0, 0, 0, true, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   1, 0, 1, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   2, 0, 2, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   3, 0, 3, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   4, 0, 4, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   5, 0, 5, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   6, 0, 6, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   7, 0, 7, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   8, 0, 8, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   9, 0, 9, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   10, 0, 10, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   11, 0, 11, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   12, 0, 12, false, 0, errMsg);
     pdbClient.addSharedPage("word2vec", "weights2", "FFMatrixBlock",
                    "word2vec", "shared_weights", "FFMatrixBlock",
                   13, 0, 13, false, 0, errMsg);

     //create private set 2
     pdbClient.createSet<FFMatrixBlock>("word2vec", "inputs", errMsg,
                     DEFAULT_PAGE_SIZE, "weights1", nullptr, nullptr, false);

         // batch_size x vocab_size
     std::cout << "To load matrix for word2vec:inputs" << std::endl;
     ff::loadMatrix(pdbClient, "word2vec", "inputs", batchSize, matrix1_totalNumBlock_y*block_y, block_x,
                   block_y, false, false, errMsg);    

     }
   

  //create output set
  pdbClient.removeSet("word2vec", "outputs", errMsg);
  pdbClient.createSet<FFMatrixBlock>("word2vec", "outputs", errMsg,
		  DEFAULT_PAGE_SIZE, "outputs", nullptr, nullptr, false);

  execute(pdbClient, "weights1");
  execute(pdbClient, "weights2");

}

#endif
