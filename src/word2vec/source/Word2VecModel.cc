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
#include "SemanticClassifier.h"
#include "SimpleFF.h"

using namespace std;

int main(int argc, char *argv[]) {

  bool reloadData = true;
  string errMsg;
  string input_path, embedding_path;
  int block_x, block_y, batch_size;

  if (argc < 3) {
    cout << "Usage: blockDimensionX blockDimensionY whetherToLoadData"
            "path/to/embedding_weights(leave empty if generate random)"
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

  bool generate = true;

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
      
      ff::createSet(pdbClient, "word2vec", "inputs", "inputs", 64);
      ff::createSet(pdbClient, "word2vec", "weights", "weights", 64);

  }
  ff::loadLibrary(pdbClient, "libraries/libSemanticClassifier.so");

  ff::createSet(pdbClient, "word2vec", "outputs", "outputs", 64);

  const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  if (!generate && reloadData) {
    input_path = string(argv[4]) + "/inputs.txt";
    embedding_path = string(argv[4]) + "/weights.txt";

    // load the input data
    (void)ff::load_matrix_data(pdbClient, input_path, "word2vec", "inputs", block_x, block_y,
                               false, false, errMsg);

    // load the embedding weights
    (void)ff::load_matrix_data(pdbClient, embedding_path, "word2vec", "weights", block_x, block_y,
                               false, false, errMsg);
  } else if (reloadData) {
    batch_size = 100;
    int vocab_size = 1000000;
    int embedding_dimension = 500;


    // batch_size x vocab_size
    std::cout << "To load matrix for word2vec:inputs" << std::endl;
    ff::loadMatrix(pdbClient, "word2vec", "inputs", batch_size, vocab_size, block_x,
                   block_y, false, false, errMsg);

    // embedding_dimension x vocab_size 
    std::cout << "To load matrix for word2vec:weights" << std::endl;
    ff::loadMatrix(pdbClient, "word2vec", "weights", embedding_dimension, vocab_size, block_x, block_y,
                   false, false, errMsg);
  }

  auto begin = std::chrono::high_resolution_clock::now();

  // make the reader
  pdb::Handle<pdb::Computation> readA =
      makeObject<FFMatrixBlockScanner>("word2vec", "weights");
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
  // pdb::Handle<pdb::Computation> myWriter = nullptr;
  // myWriter = pdb::makeObject<FFMatrixWriter>("word2vec", "outputs");
  // myWriter->setInput(myAggregation);

  bool materializeHash = true;

    // run the computation
  // if (!pdbClient.executeComputations(errMsg, "wrod2vec", materializeHash, myWriter)) {
  //   cout << "Computation failed. Message was: " << errMsg << "\n";
  //   exit(1);
  // }


  //verify the results
  // ff::print_stats(pdbClient, "word2vec", "outputs");
  // ff::print(pdbClient, "word2vec", "outputs");


  // pdb::Handle<pdb::Computation> readA1 =
  //     makeObject<FFMatrixBlockScanner>("word2vec", "outputs");


  pdb::Handle<pdb::Computation> classifier = pdb::makeObject<SemanticClassifier>();
  classifier->setInput(myAggregation);
  // pdb::Handle<pdb::Computation> classifier = pdb::makeObject<FFMatrixBlockScanner>("word2vec", "weights");

  ff::createSet(pdbClient, "word2vec", "labels", "labels", 64);
  pdb::Handle<pdb::Computation> labelWriter = nullptr;
  labelWriter = pdb::makeObject<FFMatrixWriter>("word2vec", "labels");
  labelWriter->setInput(classifier);

  auto exe_begin = std::chrono::high_resolution_clock::now();

  if (!pdbClient.executeComputations(errMsg, "wrod2vec1", materializeHash, labelWriter)) {
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

  ff::print_stats(pdbClient, "word2vec", "labels");
  return 0;
}
