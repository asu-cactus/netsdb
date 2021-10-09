
#ifndef TEST_SEMANTIC_CLASSIFICATION_CC
#define TEST_SEMANTIC_CLASSIFICATION_CC

#include "PDBString.h"
#include "PDBMap.h"
#include "DataTypes.h"
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
#include "SemanticClassifier.h"
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

void create_weights_set(pdb::PDBClient & pdbClient, std::string weight_set_name, int numBlock_x, int block_x, int totalNumBlock_y,
	int block_y) {

     std::string errMsg;
     pdbClient.removeSet("text-classification", weight_set_name, errMsg);
     //create private set 
     pdbClient.createSet<FFMatrixBlock>("text-classification", weight_set_name, errMsg,
                     DEFAULT_PAGE_SIZE, weight_set_name, nullptr, nullptr, false);

     //load blocks to the private set 
     ff::loadMatrix(pdbClient, "text-classification", weight_set_name, numBlock_x*block_x, totalNumBlock_y*block_y, block_x, block_y, false, false, errMsg);

}

void execute(pdb::PDBClient & pdbClient, std::string weight_set_name) {

  auto begin = std::chrono::high_resolution_clock::now();

  std::string errMsg;

  //create output set
  pdbClient.removeSet("text-classification", "outputs", errMsg);
  pdbClient.createSet<FFMatrixBlock>("text-classification", "outputs", errMsg,
                  DEFAULT_PAGE_SIZE, "outputs", nullptr, nullptr, false);


  // make the reader
  pdb::Handle<pdb::Computation> readA =
      makeObject<FFMatrixBlockScanner>("text-classification", weight_set_name);
  pdb::Handle<pdb::Computation> readB =
      makeObject<FFMatrixBlockScanner>("text-classification", "inputs");

  // make the transpose multiply join
  pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
  join->setInput(0, readA);
  join->setInput(1, readB);

  // make the transpose multiply aggregation
  pdb::Handle<pdb::Computation> myAggregation =
      pdb::makeObject<FFAggMatrix>();
  myAggregation->setInput(join);


  // make the classifier
  pdb::Handle<pdb::Computation> classifier = pdb::makeObject<SemanticClassifier>();
  classifier->setInput(myAggregation);

  // make the writer
  pdb::Handle<pdb::Computation> myWriter = nullptr;
  myWriter = pdb::makeObject<FFMatrixWriter>("text-classification", "outputs");
  myWriter->setInput(classifier);

  bool materializeHash = false;

  auto exe_begin = std::chrono::high_resolution_clock::now();

  // run the computation
  if (!pdbClient.executeComputations(errMsg, weight_set_name, materializeHash, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
  }

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "****Text Classification End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  std::cout << "****Text Classification Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - exe_begin).count()
              << " secs." << std::endl;

  //verify the results
  ff::print_stats(pdbClient, "text-classification", "outputs");
  ff::print(pdbClient, "text-classification", "outputs");

}


int main(int argc, char* argv[]) {

     bool loadData = true;
     if (argc > 1) {
        if (strcmp(argv[1], "N") == 0) {
           loadData = false;
	}
     }

     int numModels = 5;
     if (argc > 2) {
         numModels = atoi(argv[2]);
     }
     makeObjectAllocatorBlock(124 * 1024 * 1024, true);

    //create a shared set
     string masterIp = "localhost";
     pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("TestSharedSetLog");
     pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
     pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

     std::string errMsg;

     int block_x = 100;
     int block_y = 10000;
     int numBlock_y = 101;
     int numBlock_x = 5;

     int batchSize = 100;

     
     if (loadData) {

         ff::createDatabase(pdbClient, "text-classification");
         ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
         ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
         ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
         ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
         ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
         ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
         ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
         ff::loadLibrary(pdbClient, "libraries/libSemanticClassifier.so");

	 //create the set for storing shared weights 
         pdbClient.createSet<FFMatrixBlock>("text-classification", "shared_weights", errMsg,
                     DEFAULT_PAGE_SIZE, "weights", nullptr, nullptr, true);

         //load blocks to the shared set
         ff::loadMatrix(pdbClient, "text-classification", "shared_weights", numBlock_x*block_x, numBlock_y*block_y, block_x, block_y, false, false, errMsg);

	
         //create weights set for model 0	 
         create_weights_set(pdbClient, "weights"+std::to_string(0), numBlock_x, block_x, numBlock_y, block_y);

	 //create weights set for model 1
         create_weights_set(pdbClient, "weights"+std::to_string(1), numBlock_x, block_x, numBlock_y, block_y);

	 //create weights set for model 2
         create_weights_set(pdbClient, "weights"+std::to_string(2), numBlock_x, block_x, numBlock_y, block_y);

         //create weights set for model 3
         create_weights_set(pdbClient, "weights"+std::to_string(3), numBlock_x, block_x, numBlock_y, block_y);

	 //create weights set for model 4
         create_weights_set(pdbClient, "weights"+std::to_string(4), numBlock_x, block_x, numBlock_y, block_y);

         //create input set
         pdbClient.createSet<FFMatrixBlock>("text-classification", "inputs", errMsg,
                     DEFAULT_PAGE_SIZE, "weights1", nullptr, nullptr, false);

         // batch_size x vocab_size
         std::cout << "To load matrix for text-classification:inputs" << std::endl;
         ff::loadMatrix(pdbClient, "text-classification", "inputs", batchSize, numBlock_y*block_y, block_x,
                   block_y, false, false, errMsg);    

     }
   

     for (int i = 0; i < numModels; i++) {
         execute(pdbClient, "weights"+std::to_string(i));
     }
}

#endif
