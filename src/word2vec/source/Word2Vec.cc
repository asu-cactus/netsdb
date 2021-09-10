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

void load_matrix_from_path(pdb::PDBClient &pdbClient, string path,
                      pdb::String dbName, pdb::String setName, int totalX,
                      int totalY, int blockX, int blockY, bool dont_pad_x, 
                      bool dont_pad_y, string &errMsg, int size = 128, bool partitionByCol = true) {
  if (path.size() == 0) {
    throw runtime_error("Invalid filepath: " + path);
  }

  /// 1. Load the data from the file

  // open the input file
  ifstream is(path);
  // ignore the first line of the input file
  is.ignore(std::numeric_limits<std::streamsize>::max(),'\n');

  cout << totalX << ", " << totalY << endl;

  vector<vector<double>> matrix;

  double val;
  int total = 0;
  pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);

  pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> storeMatrix1 =
      pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

  int numXBlocks = ceil(totalX / (double)blockX);
  int numYBlocks = ceil(totalY / (double)blockY);

  int i = 0;
  int j = 0;
  int ii = 0;
  int jj = 0;

  while (i < numXBlocks) {

    try {
      while (i < numXBlocks) {
        int actual_blockX =
            dont_pad_x ? min(blockX, totalX - i * blockX) : blockX;

        while (j < numYBlocks) {
          int actual_blockY =
              dont_pad_y ? min(blockY, totalY - j * blockY) : blockY;

          pdb::Handle<FFMatrixBlock> myData =
              pdb::makeObject<FFMatrixBlock>(i, j, actual_blockX, actual_blockY,
                                             totalX, totalY, partitionByCol);

          while (ii < actual_blockX) {
            while (jj < actual_blockY) {
              int curX = (i * actual_blockX + ii);
              int curY = (j * actual_blockY + jj);

              if ((dont_pad_x && curX >= totalX) ||
                  (dont_pad_y && curY >= totalY)) {
                cout << "Shouldnt be here!" << endl;
                exit(1);
              }

              is >> val;
              while (is.peek() == ',' || is.peek() == ' ')
                is.ignore();

              double data = curX >= totalX || curY >= totalY ? 0 : val;
              (*(myData->getRawDataHandle()))[ii * actual_blockY + jj] = data;
              jj++;
            }
            ii++;
            jj = 0;
          }

          storeMatrix1->push_back(myData);
          total++;
          j++;
          ii = 0;
          jj = 0;
        }

        i++;
        j = 0;
        ii = 0;
        jj = 0;
      }
      if (!pdbClient.sendData<FFMatrixBlock>(
              pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
        cout << "Failed to send data to dispatcher server" << endl;
        exit(1);
      }
    } catch (pdb::NotEnoughSpace &e) {
      if (!pdbClient.sendData<FFMatrixBlock>(
              pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
        cout << "Failed to send data to dispatcher server" << endl;
        exit(1);
      }
      std::cout << "Dispatched " << storeMatrix1->size() << " blocks."
                << std::endl;
      pdb::makeObjectAllocatorBlock(size * 1024 * 1024, true);
      storeMatrix1 = pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();
    }
  }

  cout << setName << "(" << totalX << "x" << totalY << "): (" << numXBlocks
       << " x " << numYBlocks << ")" << total << " blocks = " << blockX << " x "
       << blockY << " each" << endl;

  // to write back all buffered records
  pdbClient.flushData(errMsg);
}

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

  bool generate = false;

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

  ff::createSet(pdbClient, "word2vec", "outputs", "outputs", 64);

  const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  if (!generate && reloadData) {
    batch_size = 100;
    int vocab_size = 1000000;
    int weight_size = 1009375;
    int embedding_dimension = 500;

    input_path = string(argv[4]) + "/inputs.txt";
    embedding_path = string(argv[4]) + "/weights.np";

    // load the input data
    std::cout << "To load matrix for word2vec:inputs" << std::endl;
    load_matrix_from_path(pdbClient, input_path, "word2vec", "inputs", batch_size, vocab_size, block_x, 
                    block_y, false, false, errMsg);

    // load the embedding weights
    std::cout << "To load matrix for word2vec:weights" << std::endl;
    load_matrix_from_path(pdbClient, embedding_path, "word2vec", "weights", embedding_dimension, weight_size, 
                    block_x, block_y, false, false, errMsg);
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
  pdb::Handle<pdb::Computation> myWriter = nullptr;
  myWriter = pdb::makeObject<FFMatrixWriter>("word2vec", "outputs");
  myWriter->setInput(myAggregation);

  bool materializeHash = true;

  auto exe_begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "wrod2vec", materializeHash, myWriter)) {
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

  return 0;
}