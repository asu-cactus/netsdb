#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "FFMatrixBlock.h"
// #include "pdb::ScanUserSet<FFMatrixBlock>.h"
#include "FFMatrixData.h"
#include "FFMatrixMeta.h"
#include "FFAggMatrix.h"
#include "FFTransposeMult.h"
#include "FFActivationBiasSum.h"
#include "FFInputLayerJoin.h"
// #include "FFMatrixWriter.h"

#include <ScanUserSet.h>
#include <WriteUserSet.h>

#include "FFMatrixUtil.h"
#include "PDBClient.h"
#include "SimpleFF.h"

string getName(string name, int ver) { return name + "_m" + to_string(ver); }
string getName1(string name, int ver) { return name + "1_m" + to_string(ver); }

void executeModel(pdb::PDBClient &pdbClient, string dbname, int modelVer) {
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
        pdb::makeObject<pdb::ScanUserSet<FFMatrixBlock>>(dbname, w1Name);
    pdb::Handle<pdb::Computation> readInputs =
        pdb::makeObject<pdb::ScanUserSet<FFMatrixBlock>>(dbname, inputName);

    pdb::Handle<pdb::Computation> joinW1Inp =
        pdb::makeObject<FFTransposeMult>();
    joinW1Inp->setInput(0, readW1);
    joinW1Inp->setInput(1, readInputs);

    // make the aggregation
    pdb::Handle<pdb::Computation> aggW1Inp = pdb::makeObject<FFAggMatrix>();
    aggW1Inp->setInput(joinW1Inp);

    pdb::Handle<pdb::Computation> readB1 =
        pdb::makeObject<pdb::ScanUserSet<FFMatrixBlock>>(dbname, b1Name);

    pdb::Handle<pdb::Computation> reluBias =
        pdb::makeObject<FFActivationBiasSum>(
            FFActivationBiasSum::SumActivation::Relu);
    reluBias->setInput(0, aggW1Inp);
    reluBias->setInput(1, readB1);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<pdb::WriteUserSet<FFMatrixBlock>>(dbname, y1Name);
    myWriter->setInput(reluBias);

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, getName("inference-1", modelVer),
                                       myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Amazon 14k Inference Stage 1 Time Duration for model version "
              << modelVer << ": "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end -
                                                                          begin)
                     .count()
              << " secs." << std::endl;

  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readY1 =
        pdb::makeObject<pdb::ScanUserSet<FFMatrixBlock>>(dbname, y1Name);
    // make the computation
    pdb::Handle<pdb::Computation> readW2 =
        pdb::makeObject<pdb::ScanUserSet<FFMatrixBlock>>(dbname, w2Name);

    pdb::Handle<pdb::Computation> joinW2A1 =
        pdb::makeObject<FFInputLayerJoin>();
    joinW2A1->setInput(0, readW2);
    joinW2A1->setInput(1, readY1);

    // make the aggregation
    pdb::Handle<pdb::Computation> aggW2A1 = pdb::makeObject<FFAggMatrix>();
    aggW2A1->setInput(joinW2A1);

    pdb::Handle<pdb::Computation> readB2 =
        pdb::makeObject<pdb::ScanUserSet<FFMatrixBlock>>(dbname, b2Name);

    pdb::Handle<pdb::Computation> sigmodBias =
        pdb::makeObject<FFActivationBiasSum>(
            FFActivationBiasSum::SumActivation::Sigmod);
    sigmodBias->setInput(0, aggW2A1);
    sigmodBias->setInput(1, readB2);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<pdb::WriteUserSet<FFMatrixBlock>>(dbname, outputName);
    myWriter->setInput(sigmodBias);

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, getName("inference-2", modelVer),
                                       myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Amazon 14k Inference State 2 Time Duration for model version "
              << modelVer << ": "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end -
                                                                          begin)
                     .count()
              << " secs." << std::endl;
  }

  pdbClient.printStorageStats(errMsg);
}

int main(int argc, char *argv[]) {
  string errMsg;
  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger =
      make_shared<pdb::PDBLogger>("AmazonclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  if (argc < 3) {
    cout << "Usage: ./RunAmazon14k loadLibraries NumModels [Model execution sequence]"
         << endl;
    exit(-1);
  }

  bool loadLibraries = strcmp(argv[1], "Y") == 0;
  int numModels = atoi(argv[2]);
  string dbName = "amazon14k";

//   ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
//   ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
  ff::loadLibrary(pdbClient, "libraries/libFFInputLayerJoin.so");
  ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
  ff::loadLibrary(pdbClient, "libraries/libFFActivationBiasSum.so");
  ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");

  for (int i = 0; i < numModels; i++) {
      string outputName = getName("output", i);
      string y1Name = getName("y1", i);
      pdbClient.removeSet(dbName, y1Name, errMsg);
      pdbClient.removeSet(dbName, outputName, errMsg);

    {
        const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

        if (!ff::is_empty_set(pdbClient, catalogClient, dbName, outputName) ||
            !ff::is_empty_set(pdbClient, catalogClient, dbName, y1Name)) {
            cout << "Old data exists!" << endl;
            exit(1);
        }
    }
  }

  for (int i = 0; i < numModels; i++) {
      string outputName = getName("output", i);
      string outputName1 = getName1("output", i);
      string y1Name = getName("y1", i);
      string y1Name1 = getName1("y1", i);
      if (!catalogClient.setExists(dbName, outputName))
        ff::createSet(pdbClient, dbName, outputName, outputName1, 64);
      if (!catalogClient.setExists(dbName, y1Name))
        ff::createSet(pdbClient, dbName, y1Name, y1Name1, 64);
  }

  if (argc == 3) {
    for (int i = 0; i < numModels; i++) {
      executeModel(pdbClient, dbName, i);
    } 
  } else {
    for (int i = 3; i < argc; i++) {
      int model_id = atoi(argv[i]);
      executeModel(pdbClient, dbName, model_id);
    } 
  }

  return 0;
}
