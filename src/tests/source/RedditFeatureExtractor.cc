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
#include "RedditCommentLabelJoin.h"
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


#include "CommentsChunk.h"
#include "CommentsToChunks.h"
#include "CommentChunkToComments.h"

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
  pdb::PDBLoggerPtr clientLogger =
      make_shared<pdb::PDBLogger>("FFRedditclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  int block_x, block_y;
  int batch_size;

  if (argc < 3) {
    cout << "Usage: blockDimensionX blockDimensionY batchSize "
            "path/to/weights/and/bias(leave empty if generate random)"
         << endl;
    exit(-1);
  }

  block_x = atoi(argv[1]);
  block_y = atoi(argv[2]);
  batch_size = atoi(argv[3]);
  cout << "Using block dimensions " << block_x << ", " << block_y << endl;

  bool generate = argc == 4;

  int total_features = 100; // Change this later

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

  ff::createSet(pdbClient, db, set, set);

  ff::createSet(pdbClient, db, "w1", "W1");
  ff::createSet(pdbClient, db, "b1", "B1");

  ff::createSet(pdbClient, db, "w2", "W2");
  ff::createSet(pdbClient, db, "b2", "B2");

  ff::createSet(pdbClient, db, "wo", "WO");
  ff::createSet(pdbClient, db, "bo", "BO");

  ff::createSet(pdbClient, db, "output", "Output");

  ff::createSet(pdbClient, db, "inference", "Inference");

  ff::createSet(pdbClient, db, "labeled_comments", "LabeledComments");

  if (!generate) {
    string main_path = string(argv[4]);
    string input_path = main_path + "/input.out";
    string w1_path = main_path + "/w1.out";
    string w2_path = main_path + "/w2.out";
    string wo_path = main_path + "/wo.out";
    string b1_path = main_path + "/b1.out";
    string b2_path = main_path + "/b2.out";
    string bo_path = main_path + "/bo.out";

    // load the input data
    (void)ff::load_matrix_data(pdbClient, w1_path, db, "w1", block_x, block_y,
                               false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, w2_path, db, "w2", block_x, block_y,
                               false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, wo_path, db, "wo", block_x, block_y,
                               false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, b1_path, db, "b1", block_x, block_y,
                               false, true, errMsg);
    (void)ff::load_matrix_data(pdbClient, b2_path, db, "b2", block_x, block_y,
                               false, true, errMsg);
    (void)ff::load_matrix_data(pdbClient, bo_path, db, "bo", block_x, block_y,
                               false, true, errMsg);
  } else {
    int hid1_size = 128;
    int hid2_size = 256;
    int num_labels = 2;

    // 128 x 9 = 128 x 9
    ff::loadMatrix(pdbClient, db, "w1", hid1_size, total_features, block_x,
                   block_y, false, false, errMsg);
    // 128 x 1
    ff::loadMatrix(pdbClient, db, "b1", hid1_size, 1, block_x, block_y, false,
                   true, errMsg);

    // 256 x 128
    ff::loadMatrix(pdbClient, db, "w2", hid2_size, hid1_size, block_x, block_y,
                   false, false, errMsg);
    // 256 x 1
    ff::loadMatrix(pdbClient, db, "b2", hid2_size, 1, block_x, block_y, false,
                   true, errMsg);

    // 2 x 256
    ff::loadMatrix(pdbClient, db, "wo", num_labels, hid2_size, block_x, block_y,
                   false, false, errMsg);
    // 2 x 1
    ff::loadMatrix(pdbClient, db, "bo", num_labels, 1, block_x, block_y, false,
                   true, errMsg);
  }

  ff::loadLibrary(pdbClient, "libraries/libRedditCommentFeatures.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditCommentFeatureChunks.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditCommentsToFeatures.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditCommentFeaturesToChunks.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditCommentChunksToBlocks.so");
  ff::loadLibrary(pdbClient, "libraries/libRedditMatrixBlockPartition.so");

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    // make the computation
    pdb::Handle<pdb::Computation> readB =
        makeObject<ScanUserSet<reddit::Comment>>(db, "comments");

    pdb::Handle<pdb::Computation> sel =
        pdb::makeObject<reddit::CommentsToFeatures>();
    sel->setInput(readB);

    pdb::Handle<pdb::Computation> chonk =
        pdb::makeObject<reddit::CommentFeaturesToChunks>(block_x);
    chonk->setInput(sel);

    pdb::Handle<pdb::Computation> slice =
        pdb::makeObject<reddit::CommentChunksToBlocks>(block_x, block_y, true,
                                                       batch_size);
    slice->setInput(chonk);


    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<reddit::MatrixBlockPartition>(db, set);
    myWriter->setInput(slice);


    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

  double dropout_rate = 0.5;
  ff::inference(pdbClient, db, "w1", "w2", "wo", set, "b1", "b2", "bo",
                "output", dropout_rate);

  ff::loadLibrary(pdbClient, "libraries/libRedditCommentInferenceJoin.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixMultiSel.so");
  ff::loadLibrary(pdbClient, "libraries/libInferenceResult.so");
  ff::loadLibrary(pdbClient, "libraries/libInferenceResultPartition.so");

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>(db, "output");

    pdb::Handle<pdb::Computation> multi_sel = makeObject<FFMatrixMultiSel>();
    multi_sel->setInput(readA);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<InferenceResultPartition>(db, "inference");
    myWriter->setInput(multi_sel);

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
        makeObject<ScanUserSet<InferenceResult>>(db, "inference");

    pdb::Handle<pdb::Computation> readB =
        makeObject<ScanUserSet<reddit::Comment>>(db, "comments");
    
    pdb::Handle<pdb::Computation> join =
        pdb::makeObject<reddit::CommentInferenceJoin>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the writer
    pdb::Handle<pdb::Computation> writer =
        pdb::makeObject<WriteUserSet<reddit::Comment>>(db, "labeled_comments");
    writer->setInput(join);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, writer)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

  pdbClient.deleteSet(db, "w1");
  pdbClient.deleteSet(db, "b1");
  pdbClient.deleteSet(db, "w2");
  pdbClient.deleteSet(db, "b2");
  pdbClient.deleteSet(db, "wo");
  pdbClient.deleteSet(db, "bo");
  pdbClient.deleteSet(db, "output");

  pdbClient.flushData(errMsg);

  // {
  //   const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 1024};

  //   auto it = pdbClient.getSetIterator<reddit::Comment>(db, "labeled_comments");

  //   for (auto r : it) {
  //     cout << r->label << endl;
  //   }
  // }

  pdbClient.registerType("libraries/libRedditComment.so", errMsg);
  pdbClient.registerType("libraries/libRedditAuthor.so", errMsg);
  pdbClient.registerType("libraries/libRedditFeatures.so", errMsg);
  pdbClient.registerType("libraries/libRedditJoin.so", errMsg);
  pdbClient.registerType("libraries/libRedditPositiveLabelSelection.so", errMsg);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};


    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<ScanUserSet<reddit::Comment>>(db, "labeled_comments");

    pdb::Handle<pdb::Computation> readB =
        makeObject<ScanUserSet<reddit::Author>>(db, "authors");

    pdb::Handle<pdb::Computation> join = makeObject<reddit::JoinAuthorsWithComments>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    Handle<Computation> myWriteSet = makeObject<WriteUserSet<reddit::Features>>("redditDB", "features");
    myWriteSet->setInput(join);


    // run the computation
    if (!pdbClient.executeComputations(errMsg, "reddit-a", myWriteSet)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }

    pdbClient.flushData(errMsg);
  }

  return 0;
}
