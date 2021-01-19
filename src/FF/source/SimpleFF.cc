#include "SimpleFF.h"

#include "FFAggMatrix.h"
#include "FFInputLayerJoin.h"
#include "FFMatrixBlock.h"
#include "FFMatrixBlockScanner.h"
#include "FFMatrixData.h"
#include "FFMatrixMeta.h"
#include "FFMatrixWriter.h"
#include "FFOutputLayer.h"
#include "FFReluBiasSum.h"
#include "FFRowAggregate.h"
#include "FFTransposeBiasSum.h"
#include "FFTransposeMult.h"

#include "FFMatrixUtil.h"
#include "PDBClient.h"

namespace ff {
void loadLibrary(pdb::PDBClient &pdbClient, string path) {
  string errMsg;
  if (!pdbClient.registerType(path, errMsg)) {
    cout << "Couldnt include " << path << ": " << errMsg << endl;
    exit(-1);
  }
}

void createSet(pdb::PDBClient &pdbClient, string dbName, string setName,
               string setName1) {

  string errMsg;
  pdbClient.removeSet(dbName, setName, errMsg);
  if (!pdbClient.createSet<FFMatrixBlock>(
          dbName, setName, errMsg, (size_t)64 * (size_t)1024 * (size_t)1024,
          setName1)) {
    cout << "Not able to create set: " + errMsg;
    //exit(-1); //It is possible that the set exists
  } else {
    cout << "Created set.\n";
  }
}


void createDatabase(pdb::PDBClient &pdbClient, string dbName) {
  string errMsg;
  pdbClient.removeDatabase(dbName, errMsg);
  if (!pdbClient.createDatabase(dbName, errMsg)) {
    cout << "Not able to create database: " << errMsg << endl;
    //exit(-1); //It is possible that the database exists.
  } else {
    cout << "Created database" << endl;
  }
}

void setup(pdb::PDBClient &pdbClient, string database) {
  loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
  loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
  loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
  loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
  loadLibrary(pdbClient, "libraries/libFFInputLayerJoin.so");
  loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
  loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
  loadLibrary(pdbClient, "libraries/libFFReluBiasSum.so");
  loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
  loadLibrary(pdbClient, "libraries/libFFTransposeBiasSum.so");
  loadLibrary(pdbClient, "libraries/libFFRowAggregate.so");
  loadLibrary(pdbClient, "libraries/libFFOutputLayer.so");

  createSet(pdbClient, database, "y1", "Y1");

  createSet(pdbClient, database, "y2", "Y2");

  createSet(pdbClient, database, "yo", "YO");
}

void inference_compute(pdb::PDBClient &pdbClient, string database, string w1,
                       string w2, string wo, string inputs, string b1,
                       string b2, string bo, double dropout_rate) {
  string errMsg;

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>(database, w1);
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>(database, inputs);

    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation =
        pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    pdb::Handle<pdb::Computation> readC =
        makeObject<FFMatrixBlockScanner>(database, b1);

    pdb::Handle<pdb::Computation> reluBias =
        pdb::makeObject<FFReluBiasSum>(dropout_rate);
    reluBias->setInput(0, myAggregation);
    reluBias->setInput(1, readC);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>(database, "y1");
    myWriter->setInput(reluBias);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>(database, w2);
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>(database, "y1");

    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFInputLayerJoin>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation =
        pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    pdb::Handle<pdb::Computation> readC =
        makeObject<FFMatrixBlockScanner>(database, b2);

    pdb::Handle<pdb::Computation> reluBias =
        pdb::makeObject<FFReluBiasSum>(dropout_rate);
    reluBias->setInput(0, myAggregation);
    reluBias->setInput(1, readC);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>(database, "y2");
    myWriter->setInput(reluBias);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

  pdbClient.deleteSet(database, "y1");

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>(database, wo);
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>(database, "y2");

    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFInputLayerJoin>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation =
        pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    pdb::Handle<pdb::Computation> readC =
        makeObject<FFMatrixBlockScanner>(database, bo);

    pdb::Handle<pdb::Computation> reluBias =
        pdb::makeObject<FFTransposeBiasSum>();
    reluBias->setInput(0, myAggregation);
    reluBias->setInput(1, readC);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>(database, "yo");
    myWriter->setInput(reluBias);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

  pdbClient.deleteSet(database, "y2");
}

void inference(pdb::PDBClient &pdbClient, string database, string w1, string w2,
               string wo, string inputs, string b1, string b2, string bo,
               string output, double dropout_rate) {
  string errMsg;
  inference_compute(pdbClient, database, w1, w2, wo, inputs, b1, b2, bo,
                    dropout_rate);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>(database, "yo");

    pdb::Handle<pdb::Computation> expSum = pdb::makeObject<FFRowAggregate>();
    expSum->setInput(readA);

    pdb::Handle<pdb::Computation> softmax = pdb::makeObject<FFOutputLayer>();
    softmax->setInput(0, readA);
    softmax->setInput(1, expSum);

    // make the writer
    pdb::Handle<pdb::Computation> sumWriter =
        pdb::makeObject<FFMatrixWriter>(database, output);
    sumWriter->setInput(softmax);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference", sumWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

  pdbClient.deleteSet(database, "yo");
}

void inference(pdb::PDBClient &pdbClient, string database, string w1, string w2,
               string wo, string inputs, string b1, string b2, string bo,
               pdb::Handle<pdb::Computation> &output, double dropout_rate) {
  string errMsg;
  inference_compute(pdbClient, database, w1, w2, wo, inputs, b1, b2, bo,
                    dropout_rate);

  // make the computation
  pdb::Handle<pdb::Computation> readA =
      makeObject<FFMatrixBlockScanner>(database, "yo");

  pdb::Handle<pdb::Computation> expSum = pdb::makeObject<FFRowAggregate>();
  expSum->setInput(readA);

  output = pdb::makeObject<FFOutputLayer>();
  output->setInput(0, readA);
  output->setInput(1, expSum);
}
} // namespace ff
