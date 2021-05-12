#include <cassert>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <vector>

#include "PDBClient.h"

#include "FFActivationBiasSum.h"
#include "FFAggMatrix.h"
#include "FFInputLayerJoin.h"
#include "FFMatrixBlock.h"
#include "FFMatrixBlockScanner.h"
#include "FFMatrixUtil.h"
#include "FFMatrixWriter.h"
#include "FFTransposeMult.h"
#include "SimpleFF.h"

#include "AbstractIndexer.h"
#include "FFPageIndexer.h"

using namespace std;
using namespace pdb;

bool createSharedSet(pdb::PDBClient &pdbClient, string db, string setName,
                     string setName1, int pageSize) {
  string errMsg;
  if (!pdbClient.createSet<FFMatrixBlock>(
          db, setName, errMsg, (size_t)pageSize * (size_t)1024 * (size_t)1024,
          setName1, nullptr, nullptr, true)) {
    cout << "Not able to create set: " + errMsg;
    // exit(-1); //It is possible that the set exists
    return false;
  }
  cout << "Created set.\n";
  return true;
}

string getName(string name, string ver) { return name + "_" + ver; }

void loadModel(pdb::PDBClient &pdbClient, string dbname, string modelVer,
               bool shareable, string path, int block_x, int block_y) {
  string errMsg;
  int batchSize;

  string input_path = path + "/input.out";
  string w1_path = path + "/w1.out";
  string w2_path = path + "/w2.out";
  string b1_path = path + "/b1.out";
  string b2_path = path + "/b2.out";

  string inputName = getName("inputs", modelVer);
  string w1Name = getName("w1", modelVer);
  string b1Name = getName("b1", modelVer);
  string y1Name = getName("y1", modelVer);
  string w2Name = getName("w2", modelVer);
  string b2Name = getName("b2", modelVer);
  string outputName = getName("output", modelVer);

  ff::createDatabase(pdbClient, dbname);

  ff::createSet(pdbClient, dbname, inputName, inputName, 64);

  if (shareable)
    createSharedSet(pdbClient, dbname, w1Name, w1Name, 64);
  else
    ff::createSet(pdbClient, dbname, w1Name, w1Name, 64);

  ff::createSet(pdbClient, dbname, b1Name, b1Name, 64);
  ff::createSet(pdbClient, dbname, y1Name, y1Name, 64);

  if (shareable)
    createSharedSet(pdbClient, dbname, w2Name, w2Name, 64);
  else
    ff::createSet(pdbClient, dbname, w2Name, w2Name, 64);

  ff::createSet(pdbClient, dbname, b2Name, b2Name, 64);
  ff::createSet(pdbClient, dbname, outputName, outputName, 64);

  // load the input data
  // batch_size x features_size = 1000 x 597540
  batchSize = ff::load_matrix_data(pdbClient, input_path, dbname, inputName,
                                   block_x, block_y, false, false, errMsg);
  // hidden_layer_1 size x features_size = <1k, 3k, 5k, 7k> x 597540
  ff::load_matrix_data(pdbClient, w1_path, dbname, w1Name, block_x, block_y,
                       false, false, errMsg);
  // labels_size x hidden_layer_1 = 14588 X <1k, 3k, 5k, 7k>
  ff::load_matrix_data(pdbClient, w2_path, dbname, w2Name, block_x, block_y,
                       false, false, errMsg);
  // hidden_layer_1 x 1
  ff::load_matrix_data(pdbClient, b1_path, dbname, b1Name, block_x, block_y,
                       false, true, errMsg);
  // labels_size x 1
  ff::load_matrix_data(pdbClient, b2_path, dbname, b2Name, block_x, block_y,
                       false, true, errMsg);
}

void executeModel(pdb::PDBClient &pdbClient, string dbname, string modelVer) {
  string errMsg;

  string inputName = getName("inputs", modelVer);
  string w1Name = getName("w1", modelVer);
  string b1Name = getName("b1", modelVer);
  string y1Name = getName("y1", modelVer);
  string w2Name = getName("w2", modelVer);
  string b2Name = getName("b2", modelVer);
  string outputName = getName("output", modelVer);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readW1 =
        makeObject<FFMatrixBlockScanner>(dbname, w1Name);
    pdb::Handle<pdb::Computation> readInputs =
        makeObject<FFMatrixBlockScanner>(dbname, inputName);

    pdb::Handle<pdb::Computation> joinW1Inp =
        pdb::makeObject<FFTransposeMult>();
    joinW1Inp->setInput(0, readW1);
    joinW1Inp->setInput(1, readInputs);

    // make the aggregation
    pdb::Handle<pdb::Computation> aggW1Inp = pdb::makeObject<FFAggMatrix>();
    aggW1Inp->setInput(joinW1Inp);

    pdb::Handle<pdb::Computation> readB1 =
        makeObject<FFMatrixBlockScanner>(dbname, b1Name);

    pdb::Handle<pdb::Computation> reluBias =
        pdb::makeObject<FFActivationBiasSum>(
            FFActivationBiasSum::SumActivation::Relu);
    reluBias->setInput(0, aggW1Inp);
    reluBias->setInput(1, readB1);

    // make the computation
    pdb::Handle<pdb::Computation> readW2 =
        makeObject<FFMatrixBlockScanner>(dbname, w2Name);

    pdb::Handle<pdb::Computation> joinW2A1 =
        pdb::makeObject<FFInputLayerJoin>();
    joinW2A1->setInput(0, readW2);
    joinW2A1->setInput(1, reluBias);

    // make the aggregation
    pdb::Handle<pdb::Computation> aggW2A1 = pdb::makeObject<FFAggMatrix>();
    aggW2A1->setInput(joinW2A1);

    pdb::Handle<pdb::Computation> readB2 =
        makeObject<FFMatrixBlockScanner>(dbname, b2Name);

    pdb::Handle<pdb::Computation> sigmodBias =
        pdb::makeObject<FFActivationBiasSum>(
            FFActivationBiasSum::SumActivation::Sigmod);
    sigmodBias->setInput(0, aggW2A1);
    sigmodBias->setInput(1, readB2);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>(dbname, outputName);
    myWriter->setInput(sigmodBias);

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, getName("inference", modelVer),
                                       myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Amazon 14k Inference Time Duration for model version "
              << modelVer << ": "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end -
                                                                          begin)
                     .count()
              << " secs." << std::endl;
  }

  pdbClient.flushData(errMsg);
}

void loadLibraries(pdb::PDBClient &pdbClient) {
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
  ff::loadLibrary(pdbClient, "libraries/libFFInputLayerJoin.so");
  ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
  ff::loadLibrary(pdbClient, "libraries/libFFActivationBiasSum.so");
  ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
  ff::loadLibrary(pdbClient, "libraries/libFFPageIndexer.so");
}

int main(int argc, char *argv[]) {
  string errMsg;
  int block_x, block_y;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger =
      make_shared<pdb::PDBLogger>("Amazon14kclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  string dbname = "amazon14k";

  if ((argc - 2) % 3 != 0) {
    cout << "Usage: blockDimensionX blockDimensionY [(modelVersion share "
            "path/to/weights/and/bias), ...]"
         << endl;
    exit(-1);
  }

  block_x = atoi(argv[1]);
  block_y = atoi(argv[2]);
  cout << "Using block dimensions " << block_x << ", " << block_y << endl;

  int model_count = (argc - 2) / 3;
  cout << "Loading " << model_count << " Models..." << endl;

  loadLibraries(pdbClient);

  // Assume user manually cleaned before running this code
  ff::createDatabase(pdbClient, dbname);

  for (int i = 0; i < model_count; i++) {
    int pos = 3 + i * 3;
    string path = string(argv[pos + 2]);
    string modelVersion = string(argv[pos]);
    bool shareable = atoi(argv[pos + 1]) == 1;

    loadModel(pdbClient, dbname, modelVersion, shareable, path, block_x,
              block_y);
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    pdb::Handle<pdb::AbstractIndexer> indexer =
        makeObject<FFPageIndexer>(10, 3);
    indexer->dump();

    if (!pdbClient.addTypeIndexer<FFMatrixBlock>(dbname, indexer)) {
      cout << "Not able to create indexer: " + errMsg;
      exit(-1);
    } else {
      cout << "Created indexer.\n";
    }
  }

  for (int i = 0; i < model_count; i++) {
    int pos = 3 + i * 3;
    string modelVersion = string(argv[pos]);
    executeModel(pdbClient, dbname, modelVersion);
  }

  return 0;
}