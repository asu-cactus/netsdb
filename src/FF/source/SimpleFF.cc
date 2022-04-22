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
#include "FFTransposeBiasSumSigmoid.h"
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

void createSet(pdb::PDBClient &pdbClient, string dbName, string setName, string setName1, int size) {
  string errMsg;
  pdbClient.removeSet(dbName, setName, errMsg);
  if (!pdbClient.createSet<FFMatrixBlock>(
          dbName, setName, errMsg, (size_t)size * (size_t)1024 * (size_t)1024,
          setName1)) {
    cout << "Not able to create set: " + errMsg;
    //exit(-1); //It is possible that the set exists
  } else {
    cout << "Created set.\n";
  }
}

void createSet(pdb::PDBClient &pdbClient, string dbName, string setName,
               string setName1, string jobName, string computationName, string lambdaName, int size) {
  string errMsg;
  pdbClient.removeSet(dbName, setName, errMsg);
  Handle<LambdaIdentifier> identifier = pdb::makeObject<LambdaIdentifier>(jobName, computationName, lambdaName);
  if (!pdbClient.createSet<FFMatrixBlock>(
          dbName, setName, errMsg, (size_t)size * (size_t)1024 * (size_t)1024,
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

static bool materializeHash = true;


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

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference-1", materializeHash, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inference-1 Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
  }

   //pdbClient.flushData (errMsg);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};
#ifdef DEBUG_SIMPLE_FF
    print_stats(pdbClient, database, w1);
    print_stats(pdbClient, database, inputs);
    print_stats(pdbClient, database, b1);
    print_stats(pdbClient, database, "y1");
#endif
#ifdef DEBUG_SIMPLE_FF_VERBOSE
    print(pdbClient, database, w1);
    print(pdbClient, database, inputs);
    print(pdbClient, database, b1);
    print(pdbClient, database, "y1");
#endif
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
    pdb::Handle<pdb::Computation> myWriter = nullptr;

    if (enablePartition)
        myWriter = pdb::makeObject<FFMatrixPartitioner>(database, "y2");
    else
        myWriter = pdb::makeObject<FFMatrixWriter>(database, "y2");
    myWriter->setInput(reluBias);

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference-2", materializeHash, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inference-2 Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
  }

   //pdbClient.flushData (errMsg);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};
#ifdef DEBUG_SIMPLE_FF
    print_stats(pdbClient, database, w2);
    print_stats(pdbClient, database, "y1");
    print_stats(pdbClient, database, b2);
    print_stats(pdbClient, database, "y2");
#endif
#ifdef DEBUG_SIMPLE_FF_VERBOSE
    print(pdbClient, database, w2);
    print(pdbClient, database, "y1");
    print(pdbClient, database, b2);
    print(pdbClient, database, "y2");
#endif
  }

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

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference-3", materializeHash, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inference-3 Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
  }

   //pdbClient.flushData (errMsg);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};
#ifdef DEBUG_SIMPLE_FF
    print_stats(pdbClient, database, wo);
    print_stats(pdbClient, database, "y2");
    print_stats(pdbClient, database, bo);
    print_stats(pdbClient, database, "yo");
#endif
#ifdef DEBUG_SIMPLE_FF_VERBOSE
    print(pdbClient, database, wo);
    print(pdbClient, database, "y2");
    print(pdbClient, database, bo);
    print(pdbClient, database, "yo");
#endif
  }
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

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference-3", materializeHash, sumWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inference-3 Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
  }
}

void inference_unit(pdb::PDBClient &pdbClient, string database, string w1,
               string wo, string inputs, string b1, string bo,
               string output, double dropout_rate, bool enablePartition) {
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

    // make the computation
    pdb::Handle<pdb::Computation> readD =
        makeObject<FFMatrixBlockScanner>(database, wo);

    pdb::Handle<pdb::Computation> join1 = pdb::makeObject<FFInputLayerJoin>();
    join1->setInput(0, readD);
    join1->setInput(1, reluBias);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation1 =
        pdb::makeObject<FFAggMatrix>();
    myAggregation1->setInput(join1);

    pdb::Handle<pdb::Computation> readE =
        makeObject<FFMatrixBlockScanner>(database, bo);

    pdb::Handle<pdb::Computation> reluBias1 =
        pdb::makeObject<FFTransposeBiasSum>();
    reluBias1->setInput(0, myAggregation1);
    reluBias1->setInput(1, readE);

    pdb::Handle<pdb::Computation> intermediateWriter =
        pdb::makeObject<FFMatrixWriter>(database, "yo");
    intermediateWriter->setInput(reluBias1);

    auto begin0 = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference-unit-intermediate", materializeHash, intermediateWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end0 = std::chrono::high_resolution_clock::now();
    std::cout << "Inference-unit Intermediate Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end0 - begin0).count()
              << " secs." << std::endl;

    pdb::Handle<pdb::Computation> readF =
        makeObject<FFMatrixBlockScanner>(database, "yo");

    pdb::Handle<pdb::Computation> expSum = pdb::makeObject<FFRowAggregate>();
    expSum->setInput(readF);

    pdb::Handle<pdb::Computation> softmax = pdb::makeObject<FFOutputLayer>();
    softmax->setInput(0, readF);
    softmax->setInput(1, expSum);

    // make the writer
    pdb::Handle<pdb::Computation> sumWriter =
        pdb::makeObject<FFMatrixWriter>(database, output);
    sumWriter->setInput(softmax);

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "inference-unit", materializeHash, sumWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inference-unit Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
  }
}


// Inference_Unit for Logistic Regression
void inference_unit_log_reg(pdb::PDBClient &pdbClient, std::string database, std::string w, std::string inputs, std::string b, std::string output, double dropout_rate, bool enablePartition) {
    string errMsg;

    {
        const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

        // read w,inputs
        pdb::Handle<pdb::Computation> read_W = makeObject<FFMatrixBlockScanner>(database, w);
        pdb::Handle<pdb::Computation> read_IN = makeObject<FFMatrixBlockScanner>(database, inputs);

        // join w,x as one compute unit for compute localization
        pdb::Handle<pdb::Computation> join_W_IN = pdb::makeObject<FFTransposeMult>();
        join_W_IN->setInput(0, read_IN); // 5000x6
        join_W_IN->setInput(1, read_W); // 1x6

        // define the computation w*x
        pdb::Handle<pdb::Computation> aggregate_W_IN = pdb::makeObject<FFAggMatrix>();
        aggregate_W_IN->setInput(join_W_IN);

        // read b
        pdb::Handle<pdb::Computation> read_B = pdb::makeObject<FFMatrixBlockScanner>(database, b);

        // define the computation: join (w*x) and b along with exp() operation on each element
        pdb::Handle<pdb::Computation> exp_of_W_IN_plus_B = pdb::makeObject<FFTransposeBiasSum>();
        exp_of_W_IN_plus_B->setInput(0, join_W_IN);  // used to be aggregate_W_IN
        exp_of_W_IN_plus_B->setInput(1, read_B);

        // compute y_intermediate = exp(w*x + b) where sigmoid operation is still pending
        pdb::Handle<pdb::Computation> aggregate_y_intermediate = pdb::makeObject<FFMatrixWriter>(database, "y");
        aggregate_y_intermediate->setInput(exp_of_W_IN_plus_B);

        auto timer_begin_y_intermediate = std::chrono::high_resolution_clock::now();
        // execute the computation
        if (!pdbClient.executeComputations(errMsg, "inference-unit-intermediate", materializeHash, aggregate_y_intermediate)) {
            cout << "Computation failed. Message was: " << errMsg << "\n";
            exit(1);
        }
        auto timer_end_y_intermediate = std::chrono::high_resolution_clock::now();
        std::cout << "Inference-unit Intermediate Stage Time Duration: " << std::chrono::duration_cast<std::chrono::duration<float>>(timer_end_y_intermediate - timer_begin_y_intermediate).count() << " secs." << std::endl;

        // read computed y_intermediate: x
        pdb::Handle<pdb::Computation> read_y_intermediate = makeObject<FFMatrixBlockScanner>(database, "y");
        cout << "Checkpoint: read_y_intermediate" << endl;
        // define sum of exponents (that is y_intermediate values) sigma(exp(x))
        pdb::Handle<pdb::Computation> sum_y_intermediate_values = makeObject<FFRowAggregate>();
        sum_y_intermediate_values->setInput(read_y_intermediate);

        pdb::Handle<pdb::Computation> read_y_intermediate_for_sigmoid = makeObject<FFMatrixBlockScanner>(database, "y");

        cout << "Checkpoint: sum_y_intermediate_values" << endl;
        // define softmax operation by setting read_y_intermediate and sum_y_intermediate_values: sigmoid=x/sigma(x)
        pdb::Handle<pdb::Computation> softmax = pdb::makeObject<FFOutputLayer>();
        softmax->setInput(0, read_y_intermediate_for_sigmoid);
        softmax->setInput(1, sum_y_intermediate_values);
        cout << "Checkpoint: softmax" << endl;

        // define the compute operation and write it to output
        pdb::Handle<pdb::Computation> output_writer = pdb::makeObject<FFMatrixWriter>(database, output);
        output_writer->setInput(softmax);
        cout << "Checkpoint: output_writer" << endl;

        auto timer_begin_y = std::chrono::high_resolution_clock::now();
        // execute the computation
        if (!pdbClient.executeComputations(errMsg, "inference-unit", materializeHash, output_writer)) {
            cout << "Computation failed. Message was: " << errMsg << "\n";
            exit(1);
        }
        auto timer_end_y = std::chrono::high_resolution_clock::now();
        cout << "Checkpoint: executeComputations" << endl;
        std::cout << "Inference-unit Stage Time Duration: " << std::chrono::duration_cast<std::chrono::duration<float>>(timer_end_y - timer_begin_y).count() << " secs." << std::endl;
    }
}

// Inference_Unit for Logistic Regression
void inference_unit_log_reg1(pdb::PDBClient &pdbClient, std::string database, std::string w, std::string inputs, std::string b, std::string output, bool enablePartition) {
    string errMsg;

    {
        const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

        // read w,inputs
        pdb::Handle<pdb::Computation> read_W = makeObject<FFMatrixBlockScanner>(database, w);
        pdb::Handle<pdb::Computation> read_IN = makeObject<FFMatrixBlockScanner>(database, inputs);

        // join w,x as one compute unit for compute localization
        pdb::Handle<pdb::Computation> join_W_IN = pdb::makeObject<FFInputLayerJoin>();
        join_W_IN->setInput(0, read_W);
        join_W_IN->setInput(1, read_IN);

        // define the computation w*x
        //pdb::Handle<pdb::Computation> aggregate_W_IN = pdb::makeObject<FFAggMatrix>();
        //aggregate_W_IN->setInput(join_W_IN);

        // read b
        pdb::Handle<pdb::Computation> read_B = pdb::makeObject<FFMatrixBlockScanner>(database, b);

        // define the computation: join (w*x) and b along with exp() operation on each element
        pdb::Handle<pdb::Computation> exp_of_W_IN_plus_B = pdb::makeObject<FFTransposeBiasSumSigmoid>();
        exp_of_W_IN_plus_B->setInput(0, join_W_IN);
        exp_of_W_IN_plus_B->setInput(1, read_B);

        // compute y_intermediate = exp(w*x + b) where sigmoid operation is still pending
        pdb::Handle<pdb::Computation> aggregate_y = pdb::makeObject<FFMatrixWriter>(database, "output");
        aggregate_y->setInput(exp_of_W_IN_plus_B);

        auto timer_begin_y = std::chrono::high_resolution_clock::now();
        // execute the computation
        if (!pdbClient.executeComputations(errMsg, "inference-unit", materializeHash, aggregate_y)) {
            cout << "Computation failed. Message was: " << errMsg << "\n";
            exit(1);
        }
        auto timer_end_y = std::chrono::high_resolution_clock::now();
        std::cout << "Inference-unit Stage Time Duration: " << std::chrono::duration_cast<std::chrono::duration<float>>(timer_end_y - timer_begin_y).count() << " secs." << std::endl;

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
