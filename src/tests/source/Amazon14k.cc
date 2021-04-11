#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "PDBClient.h"

#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "FFMatrixBlockScanner.h"
#include "FFActivationBiasSum.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFInputLayerJoin.h"
#include "SimpleFF.h"

using namespace std;
using namespace pdb;

int main(int argc, char *argv[]) {
  string errMsg;
  string input_path, w1_path, w2_path, b1_path, b2_path;
  int block_x, block_y, batch_size, hid1_size, labels_size, feature_size;

  if (argc < 2) {
    cout << "Usage: blockDimensionX blockDimensionY hiddenLayerSize labelsSize "
            "batchSize featureSize"
            "path/to/weights/and/bias(leave empty if generate random)"
         << endl;
    exit(-1);
  }

  block_x = atoi(argv[1]);
  block_y = atoi(argv[2]);
  cout << "Using block dimensions " << block_x << ", " << block_y << endl;

  hid1_size = atoi(argv[3]);
  labels_size = atoi(argv[4]);

  bool generate = argc > 6;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger =
      make_shared<pdb::PDBLogger>("Amazon14kclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
  ff::loadLibrary(pdbClient, "libraries/libFFInputLayerJoin.so");
  ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
  ff::loadLibrary(pdbClient, "libraries/libFFActivationBiasSum.so");
  ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");


  string dbname = "amazon14k";

//   pdbClient.removeDatabase(dbname, errMsg);

  pdbClient.removeSet(dbname, "inputs", errMsg);
  pdbClient.removeSet(dbname, "w1", errMsg);
  pdbClient.removeSet(dbname, "b1", errMsg);
  pdbClient.removeSet(dbname, "y1", errMsg);
  pdbClient.removeSet(dbname, "w2", errMsg);
  pdbClient.removeSet(dbname, "b2", errMsg);
  pdbClient.removeSet(dbname, "output", errMsg);
  pdbClient.flushData(errMsg);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    if (!ff::is_empty_set(pdbClient, catalogClient, dbname, "w1") ||
        !ff::is_empty_set(pdbClient, catalogClient, dbname, "inputs") ||
        !ff::is_empty_set(pdbClient, catalogClient, dbname, "b1") ||
        !ff::is_empty_set(pdbClient, catalogClient, dbname, "y1") ||
        !ff::is_empty_set(pdbClient, catalogClient, dbname, "w2") ||
        !ff::is_empty_set(pdbClient, catalogClient, dbname, "b2") ||
        !ff::is_empty_set(pdbClient, catalogClient, dbname, "output")) {
          cout << "Old data exists!" << endl;
          exit(1);
        }
  }

  ff::createDatabase(pdbClient, dbname);

  ff::createSet(pdbClient, dbname, "inputs", "inputs", 64);

  ff::createSet(pdbClient, dbname, "w1", "W1", 64);
  ff::createSet(pdbClient, dbname, "b1", "B1", 64);

  ff::createSet(pdbClient, dbname, "y1", "Y1", 64);

  ff::createSet(pdbClient, dbname, "w2", "W2", 64);
  ff::createSet(pdbClient, dbname, "b2", "B2", 64);

  ff::createSet(pdbClient, dbname, "output", "Output", 64);

  if (!generate) {
    input_path = string(argv[5]) + "/input.out";
    w1_path = string(argv[5]) + "/w1.out";
    w2_path = string(argv[5]) + "/w2.out";
    b1_path = string(argv[5]) + "/b1.out";
    b2_path = string(argv[5]) + "/b2.out";

    // load the input data
    batch_size = ff::load_matrix_data(pdbClient, input_path, dbname, "inputs",
                                      block_x, block_y, false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, w1_path, dbname, "w1", block_x,
                               block_y, false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, w2_path, dbname, "w2", block_x,
                               block_y, false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, b1_path, dbname, "b1", block_x,
                               block_y, false, true, errMsg);
    (void)ff::load_matrix_data(pdbClient, b2_path, dbname, "b2", block_x,
                               block_y, false, true, errMsg);
  } else {
    batch_size = atoi(argv[5]);
    feature_size = atoi(argv[6]);

    // X x features_size = None x 5000
    ff::loadMatrix(pdbClient, dbname, "inputs", batch_size, feature_size, block_x,
                   block_y, false, false, errMsg);

    // 128 x features_size = 128 x 5000
    ff::loadMatrix(pdbClient, dbname, "w1", hid1_size, feature_size, block_x,
                   block_y, false, false, errMsg);
    // 128 x 1
    ff::loadMatrix(pdbClient, dbname, "b1", hid1_size, 1, block_x, block_y,
                   false, true, errMsg);

    // 256 x 128
    ff::loadMatrix(pdbClient, dbname, "w2", labels_size, hid1_size, block_x,
                   block_y, false, false, errMsg);
    // 256 x 1
    ff::loadMatrix(pdbClient, dbname, "b2", labels_size, 1, block_x, block_y,
                   false, true, errMsg);
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readW1 =
        makeObject<FFMatrixBlockScanner>(dbname, "w1");
    pdb::Handle<pdb::Computation> readInputs =
        makeObject<FFMatrixBlockScanner>(dbname, "inputs");

    pdb::Handle<pdb::Computation> joinW1Inp = pdb::makeObject<FFTransposeMult>();
    joinW1Inp->setInput(0, readW1);
    joinW1Inp->setInput(1, readInputs);

    // make the aggregation
    pdb::Handle<pdb::Computation> aggW1Inp =
        pdb::makeObject<FFAggMatrix>();
    aggW1Inp->setInput(joinW1Inp);

    pdb::Handle<pdb::Computation> readB1 =
        makeObject<FFMatrixBlockScanner>(dbname, "b1");

    pdb::Handle<pdb::Computation> reluBias =
        pdb::makeObject<FFActivationBiasSum>(
            FFActivationBiasSum::SumActivation::Relu);
    reluBias->setInput(0, aggW1Inp);
    reluBias->setInput(1, readB1);

    // make the computation
    pdb::Handle<pdb::Computation> readW2 =
        makeObject<FFMatrixBlockScanner>(dbname, "w2");

    pdb::Handle<pdb::Computation> joinW2A1 = pdb::makeObject<FFInputLayerJoin>();
    joinW2A1->setInput(0, readW2);
    joinW2A1->setInput(1, reluBias);

    // make the aggregation
    pdb::Handle<pdb::Computation> aggW2A1 =
        pdb::makeObject<FFAggMatrix>();
    aggW2A1->setInput(joinW2A1);

    pdb::Handle<pdb::Computation> readB2 =
        makeObject<FFMatrixBlockScanner>(dbname, "b2");

    pdb::Handle<pdb::Computation> sigmodBias =
        pdb::makeObject<FFActivationBiasSum>(
            FFActivationBiasSum::SumActivation::Sigmod);
    sigmodBias->setInput(0, aggW2A1);
    sigmodBias->setInput(1, readB2);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>(dbname, "output");
    myWriter->setInput(sigmodBias);

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference-2", myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inference-2 Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end -
                                                                          begin)
                     .count()
              << " secs." << std::endl;
  }


  pdbClient.flushData(errMsg);

  return 0;
}