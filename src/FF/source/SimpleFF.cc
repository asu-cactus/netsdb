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
#include "FFMatrixPartitioner.h"
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

void createSet(pdb::PDBClient &pdbClient, string dbName, string setName, string setName1) {
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

void createSet(pdb::PDBClient &pdbClient, string dbName, string setName,
               string setName1, string jobName, string computationName, string lambdaName) {
  string errMsg;
  pdbClient.removeSet(dbName, setName, errMsg);
  Handle<LambdaIdentifier> identifier = pdb::makeObject<LambdaIdentifier>(jobName, computationName, lambdaName);
  if (!pdbClient.createSet<FFMatrixBlock>(
          dbName, setName, errMsg, (size_t)64 * (size_t)1024 * (size_t)1024,
          setName1, nullptr, identifier)) {
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
}

void cleanup(pdb::PDBClient &pdblient, string database) {
  string errMsg;
  // pdblient.removeSet(database, "y1", errMsg);
  // pdblient.removeSet(database, "y2", errMsg);
  // pdblient.removeSet(database, "yo", errMsg);
}

void inference_compute(pdb::PDBClient &pdbClient, string database, string w1,
                       string w2, string wo, string inputs, string b1,
                       string b2, string bo, double dropout_rate, bool enablePartition) {
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
    
    pdb::Handle<pdb::Computation> myWriter = nullptr;

    if (enablePartition) 
        myWriter = pdb::makeObject<FFMatrixPartitioner>(database, "y1");
    else
        myWriter = pdb::makeObject<FFMatrixWriter>(database, "y1");
    myWriter->setInput(reluBias);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference-1", myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

   pdbClient.flushData (errMsg);

#ifdef DEBUG_SIMPLE_FF
  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, database, w1);
    print(pdbClient, database, w1);
    print_stats(pdbClient, database, inputs);
    print(pdbClient, database, inputs);
    print_stats(pdbClient, database, b1);
    print(pdbClient, database, b1);
    print_stats(pdbClient, database, "y1");
    print(pdbClient, database, "y1");
  }
#endif

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
    pdb::Handle<pdb::Computation> myWriter = nullptr;

    if (enablePartition)
        myWriter = pdb::makeObject<FFMatrixPartitioner>(database, "y2");
    else
        myWriter = pdb::makeObject<FFMatrixWriter>(database, "y2");
    myWriter->setInput(reluBias);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference-2", myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

   pdbClient.flushData (errMsg);

#ifdef DEBUG_SIMPLE_FF
  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, database, w2);
    print(pdbClient, database, w2);
    print_stats(pdbClient, database, "y1");
    print(pdbClient, database, "y1");
    print_stats(pdbClient, database, b2);
    print(pdbClient, database, b2);
    print_stats(pdbClient, database, "y2");
    print(pdbClient, database, "y2");
  }
#endif

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
    if (!pdbClient.executeComputations(errMsg, "inference-3", myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

   pdbClient.flushData (errMsg);

#ifdef DEBUG_SIMPLE_FF
  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, database, wo);
    print(pdbClient, database, wo);
    print_stats(pdbClient, database, "y2");
    print(pdbClient, database, "y2");
    print_stats(pdbClient, database, bo);
    print(pdbClient, database, bo);
    print_stats(pdbClient, database, "yo");
    print(pdbClient, database, "yo");
  }
#endif
}

void inference(pdb::PDBClient &pdbClient, string database, string w1, string w2,
               string wo, string inputs, string b1, string b2, string bo,
               string output, double dropout_rate, bool enablePartition) {
  string errMsg;
  inference_compute(pdbClient, database, w1, w2, wo, inputs, b1, b2, bo,
                    dropout_rate, enablePartition);

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
    if (!pdbClient.executeComputations(errMsg, "inference-4", sumWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }
}

void inference(pdb::PDBClient &pdbClient, string database, string w1, string w2,
               string wo, string inputs, string b1, string b2, string bo,
               pdb::Handle<pdb::Computation> &output, double dropout_rate, bool enablePartition) {
  string errMsg;
  inference_compute(pdbClient, database, w1, w2, wo, inputs, b1, b2, bo,
                    dropout_rate, enablePartition);

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
