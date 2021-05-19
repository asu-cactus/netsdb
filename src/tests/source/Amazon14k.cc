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
  cout << "Created set " << setName << endl;
  return true;
}

string getName(string name, int ver) { return name + "_m" + to_string(ver); }

void executeModel(pdb::PDBClient &pdbClient, string dbname, int modelVer) {
  string errMsg;

  string inputName = getName("inputs", modelVer);
  string w1Name = getName("w1", modelVer);
  string b1Name = getName("b1", modelVer);
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

  // {
  //   const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};
  //   ff::print_stats(pdbClient, dbname, inputName);
  //   ff::print_stats(pdbClient, dbname, w1Name);
  //   ff::print_stats(pdbClient, dbname, w2Name);
  //   ff::print_stats(pdbClient, dbname, outputName);
  // }
}

void loadLibraries(pdb::PDBClient &pdbClient) {
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
  ff::loadLibrary(pdbClient, "libraries/libFFPageIndexer.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
  ff::loadLibrary(pdbClient, "libraries/libFFInputLayerJoin.so");
  ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
  ff::loadLibrary(pdbClient, "libraries/libFFActivationBiasSum.so");
  ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
}

void clearOutput(pdb::PDBClient &pdbClient, pdb::CatalogClient &catalogClient,
                string dbname, int num_models) {
  string errMsg;

  for (int i = 0; i < num_models; i++) {
    string outputName = getName("output", i);
    pdbClient.removeSet(dbname, outputName, errMsg);
    pdbClient.flushData(errMsg);

    if (!ff::is_empty_set(pdbClient, catalogClient, dbname, outputName)) {
      cout << "Could not clear output '" << outputName << "'!" << endl;
      exit(1);
    }

    ff::createSet(pdbClient, dbname, outputName, outputName, 64);
  }

}

void loadModels(pdb::PDBClient &pdbClient, pdb::CatalogClient &catalogClient,
                string dbname, int block_x, int block_y, int num_models,
                int batch_size, int label_size, int feature_size, int hd1size,
                bool share) {
  string errMsg;

  clearOutput(pdbClient, catalogClient, dbname, num_models);

  for (int i = 0; i < num_models; i++) {
    string inputName = getName("inputs", i);
    string w1Name = getName("w1", i);
    string b1Name = getName("b1", i);
    string w2Name = getName("w2", i);
    string b2Name = getName("b2", i);

    pdbClient.removeSet(dbname, inputName, errMsg);
    pdbClient.removeSet(dbname, w1Name, errMsg);
    pdbClient.removeSet(dbname, b1Name, errMsg);
    pdbClient.removeSet(dbname, w2Name, errMsg);
    pdbClient.removeSet(dbname, b2Name, errMsg);
    pdbClient.flushData(errMsg);

    {
      const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

      if (!ff::is_empty_set(pdbClient, catalogClient, dbname, inputName) ||
          !ff::is_empty_set(pdbClient, catalogClient, dbname, w1Name) ||
          !ff::is_empty_set(pdbClient, catalogClient, dbname, b1Name) ||
          !ff::is_empty_set(pdbClient, catalogClient, dbname, w2Name) ||
          !ff::is_empty_set(pdbClient, catalogClient, dbname, b2Name)) {
        cout << "Old data exists!" << endl;
        exit(1);
      }
    }

    ff::createSet(pdbClient, dbname, inputName, inputName, 64);
    ff::createSet(pdbClient, dbname, w1Name, w1Name, 64);
    ff::createSet(pdbClient, dbname, b1Name, b1Name, 64);
    ff::createSet(pdbClient, dbname, b2Name, b2Name, 64);
    if (share) {
      createSharedSet(pdbClient, dbname, w2Name, w2Name, 64);
    } else {
      ff::createSet(pdbClient, dbname, w2Name, w2Name, 64);
    }
  }

  if (share) {
    vector<string> setNames;
    for (int i = 0; i < num_models; i++) {
      setNames.push_back(getName("w2", i));
    }
    ff::loadMatrix(pdbClient, dbname, setNames, label_size, hd1size, block_x,
                   block_y, false, false, errMsg);
  }

  for (int i = 0; i < num_models; i++) {
    string inputName = getName("inputs", i);
    string w1Name = getName("w1", i);
    string b1Name = getName("b1", i);
    string w2Name = getName("w2", i);
    string b2Name = getName("b2", i);
    string outputName = getName("output", i);

    // batch_size x features_size = 1000 x 597540
    ff::loadMatrix(pdbClient, dbname, inputName, batch_size, feature_size,
                   block_x, block_y, false, false, errMsg);
    // hidden_layer_1 size x features_size = <1k, 3k, 5k, 7k> x 597540
    ff::loadMatrix(pdbClient, dbname, w1Name, hd1size, feature_size, block_x,
                   block_y, false, false, errMsg);
    // labels_size x hidden_layer_1 = 14588 X <1k, 3k, 5k, 7k>
    if (!share) {
      ff::loadMatrix(pdbClient, dbname, w2Name, label_size, hd1size, block_x,
                     block_y, false, false, errMsg);
    }
    // hidden_layer_1 x 1
    ff::loadMatrix(pdbClient, dbname, b1Name, hd1size, 1, block_x, block_y,
                   false, true, errMsg);
    // labels_size x 1
    ff::loadMatrix(pdbClient, dbname, b2Name, label_size, 1, block_x, block_y,
                   false, true, errMsg);
  }
}

int main(int argc, char *argv[]) {
  string errMsg;
  int block_x, block_y, numModels, batchSize, labelSize, featureSize, hd1Size;
  bool loadLibs, createDb, shareable;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger =
      make_shared<pdb::PDBLogger>("Amazon14kclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  string dbname = "amazon14k";

  cout << argc << endl;

  if (argc == 2) {
    loadLibs = string(argv[1]).compare("1") == 0;
    if (loadLibs) {
      cout << "Loading Libraries and exiting..." << endl;
      loadLibraries(pdbClient);
      return 0;
    }
  }

  if (argc != 10 && argc != 3) {
    cout << "Usage: (loadLibraries numModels [blockDimensionX blockDimensionY "
            "share batchSize labelSize featureSize hiddenLayerSize]) | loadLibraries"
         << endl;
    exit(-1);
  }

  loadLibs = atoi(argv[1]) == 1;
  createDb = argc == 10;
  numModels = atoi(argv[2]);

  if (argc == 10) {
    block_x = atoi(argv[3]);
    block_y = atoi(argv[4]);
    cout << "Using block dimensions " << block_x << ", " << block_y << endl;

    shareable = atoi(argv[5]) == 1;
    batchSize = atoi(argv[6]);
    labelSize = atoi(argv[7]);
    featureSize = atoi(argv[8]);
    hd1Size = atoi(argv[9]);

    cout << "Loading " << numModels << " Models with batchSize: " << batchSize
        << ", labelsSize: " << labelSize << ", featureSize: " << featureSize
        << ", hiddenLayerSize: " << hd1Size << endl;

    if (!shareable)
      cout << "NOT SHARING!" << endl;
    else
      cout << "SHARING!" << endl;
  }

  if (loadLibs)
    loadLibraries(pdbClient);

  if (createDb) {
    cout << "Re-creating Database" << endl;
    ff::createDatabase(pdbClient, dbname);
  }

  // Load the page indexer
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

  if (createDb) {
    cout << "Generating models..." << endl;
    // Load the models
    loadModels(pdbClient, catalogClient, dbname, block_x, block_y, numModels,
              batchSize, labelSize, featureSize, hd1Size, shareable);
  } else {
    cout << "Clearing output..." << endl;
    clearOutput(pdbClient, catalogClient, dbname, numModels);
  }

  // Execute the models
  for (int i = 0; i < numModels; i++) {
    executeModel(pdbClient, dbname, i);
  }

  return 0;
}