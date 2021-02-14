#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "FFMatrixBlock.h"
#include "FFMatrixBlockScanner.h"
#include "FFMatrixData.h"
#include "FFMatrixMeta.h"
#include "FFMatrixWriter.h"

#include "RedditComment.h"
#include <ScanUserSet.h>
#include <WriteUserSet.h>

#include "FFMatrixUtil.h"
#include "PDBClient.h"
#include "SimpleFF.h"

#include "CommentChunksToBlocks.h"
#include "CommentFeatureChunks.h"
#include "CommentFeatures.h"
#include "CommentFeaturesToChunks.h"
#include "CommentsToFeatures.h"
#include "MatrixBlockPartition.h"
#include "CommentBlockToMatrix.h"

#include "FFMatrixMultiSel.h"
#include "RedditCommentInferenceJoin.h"

#include "InferenceResult.h"
#include "InferenceResultPartition.h"

#include <RedditAuthor.h>
#include <RedditFeatures.h>
#include <RedditJoin.h>
#include <RedditPositiveLabelSelection.h>
#include <ScanUserSet.h>
#include <WriteUserSet.h>

int main(int argc, char *argv[]) {
  string errMsg;
  string masterIp = "localhost";
  string commentsSetName;
  pdb::PDBLoggerPtr clientLogger =
      make_shared<pdb::PDBLogger>("FFRedditclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  int block_x, block_y;
  int batch_size;

  if (argc < 5) {
    cout << "Usage: commentsSetName enablePartition blockDimensionX blockDimensionY batchSize path/to/weights/and/bias(leave empty if generate random)"
         << endl;
    exit(-1);
  }

  bool enablePartition = false;
  
  long totalTime = 0;


  commentsSetName = std::string(argv[1]);
  if (strcmp(argv[2], "Y")==0) {
     enablePartition = true;
  }
  block_x = atoi(argv[3]);
  block_y = atoi(argv[4]);
  batch_size = atoi(argv[5]);
  cout << "Batch Size: " << batch_size << endl;
  cout << "Using block dimensions " << block_x << ", " << block_y << endl;

  bool generate = argc == 6;

  int total_features = reddit::NUM_FEATURES; // Change this later

  string db = "redditDB", set = "comment_features";

  // ff::setup(pdbClient, db);

  ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
  ff::loadLibrary(pdbClient, "libraries/libFFInputLayerJoin.so");
  ff::loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");
  ff::loadLibrary(pdbClient, "libraries/libFFReluBiasSum.so");
  ff::loadLibrary(pdbClient, "libraries/libFFTransposeMult.so");
  ff::loadLibrary(pdbClient, "libraries/libFFTransposeBiasSum.so");
  ff::loadLibrary(pdbClient, "libraries/libFFRowAggregate.so");
  ff::loadLibrary(pdbClient, "libraries/libFFOutputLayer.so");

  pdbClient.removeSet(db, "w1", errMsg);
  pdbClient.removeSet(db, "b1", errMsg);
  pdbClient.removeSet(db, "y1", errMsg);
  pdbClient.removeSet(db, "w2", errMsg);
  pdbClient.removeSet(db, "b2", errMsg);
  pdbClient.removeSet(db, "y2", errMsg);
  pdbClient.removeSet(db, "wo", errMsg);
  pdbClient.removeSet(db, "bo", errMsg);
  pdbClient.removeSet(db, "yo", errMsg);
  pdbClient.removeSet(db, "output", errMsg);
  pdbClient.removeSet(db, "inference", errMsg);
  pdbClient.removeSet(db, "labeled_comments", errMsg);
  pdbClient.removeSet(db, "comment_features", errMsg);
  pdbClient.flushData(errMsg);

  ff::cleanup(pdbClient, db);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    if (!ff::is_empty_set(pdbClient, catalogClient, db, "w1") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "b1") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "y1") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "w2") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "b2") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "y2") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "wo") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "bo") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "yo") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "output") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "inference") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "labeled_comments") ||
        !ff::is_empty_set(pdbClient, catalogClient, db, "comment_features")) {
          cout << "Old data exists!" << endl;
          exit(1);
        }
  }

  //specify the partitioning condition here
  
  if (enablePartition) {
      // std::string loadJobId = set;
      // std::string jobName = "inference-1";
      // std::string computationName = "JoinComp_2";
      // std::string lambdaName = "methodCall_1"; 
      // std::string errMsg;
      // Handle<LambdaIdentifier> identifier = pdb::makeObject<LambdaIdentifier>(jobName, computationName, lambdaName);
      // pdbClient.createSet<FFMatrixBlock>(db, set, errMsg, 64*1024*1024, loadJobId, nullptr, identifier);//getBlockColIndex

      
      //1000 comments 200000 features

      ff::createSet(pdbClient, db, set, "CommentFeatures", "inference-1", "JoinComp_2", "methodCall_1", 64);
  }
  else
      ff::createSet(pdbClient, db, set, set, 64);

  if (!enablePartition) {
      //1000 hid1 200000 features
      ff::createSet(pdbClient, db, "w1", "W1", 32);
      //1000 x 1
      ff::createSet(pdbClient, db, "b1", "B1", 1);
      //200000 features 1000 comments
      ff::createSet(pdbClient, db, "y1", "Y1", 32);
      //2000 hid2 200000 features
      ff::createSet(pdbClient, db, "w2", "W2", 64);
      //2000 x 1
      ff::createSet(pdbClient, db, "b2", "B2", 1);
      //200000 features 1000 comments
      ff::createSet(pdbClient, db, "y2", "Y2", 32);

      ff::createSet(pdbClient, db, "wo", "WO", 1);
      ff::createSet(pdbClient, db, "bo", "BO", 1);
      ff::createSet(pdbClient, db, "yo", "YO", 1);
  } else {
      ff::createSet(pdbClient, db, "w1", "W1", "inference-1", "JoinComp_2", "methodCall_0", 32);//getBlockColIndex
      ff::createSet(pdbClient, db, "b1", "B1", "inference-1", "JoinComp_5", "methodCall_1", 1);//getBlockRowIndex
      ff::createSet(pdbClient, db, "y1", "Y1", "inference-2", "JoinComp_2", "methodCall_1", 32);//getBlockRowIndex

      ff::createSet(pdbClient, db, "w2", "W2", "inference-2", "JoinComp_2", "methodCall_0", 64);//getBlockColIndex
      ff::createSet(pdbClient, db, "b2", "B2", "inference-2", "JoinComp_5", "methodCall_1", 1);//getRowIndex
      ff::createSet(pdbClient, db, "y2", "Y2", "inference-3", "JoinComp_2", "methodCall_1", 32);//getBlockRowIndex

      ff::createSet(pdbClient, db, "wo", "WO", "inference-3", "JoinComp_2", "methodCall_0", 1);//getBlockColIndex
      ff::createSet(pdbClient, db, "bo", "BO", "inference-3", "JoinComp_5", "methodCall_1", 1);//getBlockRowIndex
      ff::createSet(pdbClient, db, "yo", "YO", 1);
  }
  //partitioning condition
  if (enablePartition) {
      std::string loadJobId = "Output";
      std::string jobName = "labeling";
      std::string computationName = "JoinComp_2";
      std::string lambdaName = "methodCall_0";
      std::string errMsg;
      Handle<LambdaIdentifier> identifier = pdb::makeObject<LambdaIdentifier>(jobName, computationName, lambdaName);
      pdbClient.createSet<InferenceResult>(db, "output", errMsg, 1*1024*1024, loadJobId, nullptr, identifier); //getKey
  } else
      ff::createSet(pdbClient, db, "output", "Output", 64);

  ff::createSet(pdbClient, db, "inference", "Inference", 64);

  ff::createSet(pdbClient, db, "labeled_comments", "LabeledComments", 64);//TO be partitioned


  if (!generate) {
    string main_path = string(argv[6]);
    string input_path = main_path + "/input.out";
    string w1_path = main_path + "/w1.out";
    string w2_path = main_path + "/w2.out";
    string wo_path = main_path + "/wo.out";
    string b1_path = main_path + "/b1.out";
    string b2_path = main_path + "/b2.out";
    string bo_path = main_path + "/bo.out";

    // load the input data
    (void)ff::load_matrix_data(pdbClient, w1_path, db, "w1", block_x, block_y,
                               false, false, errMsg, true);
    (void)ff::load_matrix_data(pdbClient, w2_path, db, "w2", block_x, block_y,
                               false, false, errMsg, true);
    (void)ff::load_matrix_data(pdbClient, wo_path, db, "wo", block_x, block_y,
                               false, false, errMsg, true);
    (void)ff::load_matrix_data(pdbClient, b1_path, db, "b1", block_x, block_y,
                               false, true, errMsg, false);
    (void)ff::load_matrix_data(pdbClient, b2_path, db, "b2", block_x, block_y,
                               false, true, errMsg, false);
    (void)ff::load_matrix_data(pdbClient, bo_path, db, "bo", block_x, block_y,
                               false, true, errMsg, false);
  } else {
    int hid1_size = 1000;
    int hid2_size = 2000;
    int num_labels = 2;

    // 1000 x 200000 = 1000 x 200000
    ff::loadMatrix(pdbClient, db, "w1", hid1_size, total_features, block_x,

                   block_y, false, false, errMsg, 64, true);
    // 1000 x 1
    ff::loadMatrix(pdbClient, db, "b1", hid1_size, 1, block_x, block_y, false,
                   true, errMsg, 64, false);

    // 2000 x 1000
    ff::loadMatrix(pdbClient, db, "w2", hid2_size, hid1_size, block_x, block_y,
                   false, false, errMsg, 32, true);
    // 2000 x 1
    ff::loadMatrix(pdbClient, db, "b2", hid2_size, 1, block_x, block_y, false,
                   true, errMsg, 8, false);

    // 2 x 1000
    ff::loadMatrix(pdbClient, db, "wo", num_labels, hid2_size, block_x, block_y,
                   false, false, errMsg, 8, true);
    // 2 x 1
    ff::loadMatrix(pdbClient, db, "bo", num_labels, 1, block_x, block_y, false,
                   true, errMsg, 8, false);
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    ff::print_stats(pdbClient, db, "w1");
    ff::print_stats(pdbClient, db, "w2");
    ff::print_stats(pdbClient, db, "wo");
    ff::print_stats(pdbClient, db, "b1");
    ff::print_stats(pdbClient, db, "b2");
    ff::print_stats(pdbClient, db, "bo");
#ifdef DEBUG_SIMPLE_FF_VERBOSE
    ff::print(pdbClient, db, "w1");
    ff::print(pdbClient, db, "w2");
    ff::print(pdbClient, db, "wo");
    ff::print(pdbClient, db, "b1");
    ff::print(pdbClient, db, "b2");
    ff::print(pdbClient, db, "bo");
#endif
  }

  ff::loadLibrary(pdbClient, "libraries/libRedditCommentFeatures.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditCommentFeatureChunks.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditCommentsToFeatures.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditCommentFeaturesToChunks.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditCommentChunksToBlocks.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditMatrixBlockPartition.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditCommentBlockToMatrix.so");



  {

    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    // make the computation
    pdb::Handle<pdb::Computation> readB =
        makeObject<ScanUserSet<reddit::Comment>>(db, commentsSetName);

    pdb::Handle<pdb::Computation> sel =
        pdb::makeObject<reddit::CommentsToFeatures>();
    sel->setInput(readB);

    pdb::Handle<pdb::Computation> chonk =
        pdb::makeObject<reddit::CommentFeaturesToChunks>(block_y);
    chonk->setInput(sel);

    pdb::Handle<pdb::Computation> slice =
        pdb::makeObject<reddit::CommentChunksToBlocks>(block_x);
    slice->setInput(chonk);

    pdb::Handle<pdb::Computation> block =
        pdb::makeObject<reddit::CommentBlockToMatrix>(block_x, block_y, true,
                                                       batch_size, total_features);
    block->setInput(slice);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = nullptr;

    if (enablePartition)
        myWriter = pdb::makeObject<reddit::MatrixBlockPartition>(db, set, MatrixBlockPartitionType::Column);
    else
        myWriter = pdb::makeObject<WriteUserSet<FFMatrixBlock>>(db, set);

    myWriter->setInput(block);

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "blocking-0", myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Blocking Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

    SetIterator<FFMatrixBlock> result = pdbClient.getSetIterator<FFMatrixBlock>(db, set);
    int count = 0;
    for (const auto &r : result) {
#ifdef DEBUG_SIMPLE_FF_VERBOSE
      r->print();  
#endif
      count++;
    }
    std::cout << "count: " << count << std::endl;
  }

  pdbClient.flushData (errMsg);

  ff::loadLibrary(pdbClient, "libraries/libRedditCommentInferenceJoin.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixMultiSel.so");
  ff::loadLibrary(pdbClient, "libraries/libInferenceResult.so");
  ff::loadLibrary(pdbClient, "libraries/libInferenceResultPartition.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixPartitioner.so");
  // Run FF inference on comment feature blocks
  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    double dropout_rate = 0;
    pdb::Handle<pdb::Computation> inference;
    auto begin = std::chrono::high_resolution_clock::now(); 
    ff::inference(pdbClient, db, "w1", "w2", "wo", set, "b1", "b2", "bo",
                  inference, dropout_rate, enablePartition);
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Inference Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

    pdb::Handle<pdb::Computation> multi_sel = makeObject<FFMatrixMultiSel>();
    multi_sel->setInput(inference);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = nullptr;

    if (enablePartition)
        myWriter = pdb::makeObject<InferenceResultPartition>(db, "output");
    else
        myWriter = pdb::makeObject<WriteUserSet<InferenceResult>>(db, "output");

    myWriter->setInput(multi_sel);


    begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "flattening", myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Inference results flatterning Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
  }

  pdbClient.flushData (errMsg);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    ff::print_stats(pdbClient, db, "yo");
#ifdef DEBUG_SIMPLE_FF_VERBOSE
    ff::print(pdbClient, db, "yo");
#endif
    
    auto it = pdbClient.getSetIterator<InferenceResult>(db, "output");
    int count = 0;

    for (auto r : it) {
#ifdef DEBUG_SIMPLE_FF_VERBOSE
      cout << "[PRINT] output : " << r->getKey() << ",0; Block Size: 1,2" << endl;
#endif
      count++;
    }
    cout << "count: " << count << endl;
  }

  // Join the partitioned inference results with comments
  {

    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<ScanUserSet<InferenceResult>>(db, "output");

    pdb::Handle<pdb::Computation> readB =
        makeObject<ScanUserSet<reddit::Comment>>(db, commentsSetName);

    pdb::Handle<pdb::Computation> join =
        pdb::makeObject<reddit::CommentInferenceJoin>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the writer
    pdb::Handle<pdb::Computation> writer =
        pdb::makeObject<WriteUserSet<reddit::Comment>>(db, "labeled_comments");
    writer->setInput(join);

    auto begin = std::chrono::high_resolution_clock::now();
    // run the computation
    if (!pdbClient.executeComputations(errMsg, "labeling", writer)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Labeling Stage Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
  }

  pdbClient.flushData(errMsg);

   {
     const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 1024};

     auto it = pdbClient.getSetIterator<reddit::Comment>(db, "labeled_comments");

    int count = 0;
    for (auto r : it) {
#ifdef DEBUG_SIMPLE_FF_VERBOSE
      cout << r->label << endl;
#endif
      count++;
    }
    std::cout << "count = " << count << std::endl;
   }

  // pdbClient.registerType("libraries/libRedditComment.so", errMsg);
  // pdbClient.registerType("libraries/libRedditAuthor.so", errMsg);
  // pdbClient.registerType("libraries/libRedditFeatures.so", errMsg);
  // pdbClient.registerType("libraries/libRedditJoin.so", errMsg);
  // pdbClient.registerType("libraries/libRedditPositiveLabelSelection.so", errMsg);

  // {
  //   const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};


  //   // make the computation
  //   pdb::Handle<pdb::Computation> readA =
  //       makeObject<ScanUserSet<reddit::Comment>>(db, "labeled_comments");

  //   pdb::Handle<pdb::Computation> readB =
  //       makeObject<ScanUserSet<reddit::Author>>(db, "authors");

  //   pdb::Handle<pdb::Computation> join = makeObject<reddit::JoinAuthorsWithComments>();
  //   join->setInput(0, readA);
  //   join->setInput(1, readB);

  //   Handle<Computation> myWriteSet = makeObject<WriteUserSet<reddit::Features>>("redditDB", "features");
  //   myWriteSet->setInput(join);

  //   auto begin = std::chrono::high_resolution_clock::now();
  //   // run the computation
  //   if (!pdbClient.executeComputations(errMsg, myWriteSet)) {
  //     cout << "Computation failed. Message was: " << errMsg << "\n";
  //     exit(1);
  //   }
  //   auto end = std::chrono::high_resolution_clock::now();
  //   std::cout << "Adaptive Join Stage Time Duration: "
  //             << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
  //             << " secs." << std::endl;
  //   pdbClient.flushData(errMsg);
  // }

  return 0;
}
