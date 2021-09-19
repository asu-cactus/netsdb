#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "PDBClient.h"
#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"

using namespace std;

void gen_model_weights(PDBClient & pdbClient, std::string weight_set_name, std::string rootPath, bool generate, bool reloadData, int block_x, int block_y, int embedding_dimension, int vocab_size) {

  std::string errMsg;

  if (!generate && reloadData) {
     
     ff::createSet(pdbClient, "word2vec", weight_set_name, weight_set_name, 256);

     std::string embedding_path = rootPath + "/" + weight_set_name + ".txt";

     // load the embedding weights
     (void)ff::load_matrix_data(pdbClient, embedding_path, "word2vec", weight_set_name, block_x, block_y,
                               false, false, errMsg);

  } else if (reloadData) {

    ff::createSet(pdbClient, "word2vec", weight_set_name, weight_set_name, 256);

    // embedding_dimension x vocab_size 
    std::cout << "To load matrix for word2vec:"<< weight_set_name << std::endl;
    ff::loadMatrix(pdbClient, "word2vec", weight_set_name, embedding_dimension, vocab_size, block_x, block_y,
                   false, false, errMsg);
  }

}

void execute_model(PDBClient & pdbClient, std::string weight_set_name) {

  std::string errMsg;

  auto begin = std::chrono::high_resolution_clock::now();

  ff::createSet(pdbClient, "word2vec", "outputs", "outputs", 256);
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

int main(int argc, char *argv[]) {

  bool reloadData = true;
  string errMsg;
  string input_path, embedding_path;
  int block_x, block_y;
  int numModels = 1;
  int batch_size = 100;
  int vocab_size = 1000000;
  int embedding_dimension = 500;

  if (argc < 3) {
    cout << "Usage: blockDimensionX blockDimensionY whetherToLoadData numModels"
            " path/to/embedding_weights(leave empty if generate random)"
         << endl;
    exit(-1);
  }

  block_x = atoi(argv[1]);
  block_y = atoi(argv[2]);

  if (argc >= 4) {
      if (strcmp(argv[3], "N")==0) {
          reloadData = false;
          std::cout << "WARNING: we do not reload data!" << std::endl;
      }
  }
  cout << "Using block dimensions " << block_x << ", " << block_y << endl;

  if (argc >=5) {
  
      numModels = atoi(argv[4]);

  }
  bool generate = true;

  std::string rootPath="";

  if (argc >=6) {
  
	  generate = false;
          rootPath = argv[5];
  
  }

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("Word2VecClientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  if (reloadData) {
      ff::createDatabase(pdbClient, "word2vec");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
      ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
      ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
      ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
      ff::createSet(pdbClient, "word2vec", "inputs", "inputs", 256);

  }


  const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 1024};

  if (!generate && reloadData) {
    input_path = string(argv[5]) + "/inputs.txt";

    // load the input data
        (void)ff::load_matrix_data(pdbClient, input_path, "word2vec", "inputs", block_x, block_y,
                               false, false, errMsg);

  } else if (reloadData) {

    // embedding_dimension x vocab_size 
    std::cout << "To load matrix for word2vec:inputs" << std::endl;
    ff::loadMatrix(pdbClient, "word2vec", "inputs", batch_size, vocab_size, block_x, block_y,
                   false, false, errMsg);
  }

  for (int i = 0; i < numModels; i++) {
      gen_model_weights(pdbClient, "weights"+std::to_string(i),rootPath, generate, reloadData, block_x, block_y, embedding_dimension, vocab_size);
  }

  for (int i = 0; i < numModels; i++) {
      execute_model(pdbClient, "weights"+std::to_string(i));
  }

  return 0;
}
