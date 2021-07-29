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

#include "CommentBlockToMatrix.h"
#include "CommentChunksToBlocks.h"
#include "CommentFeatureChunks.h"
#include "CommentFeatures.h"
#include "CommentFeaturesToChunks.h"
#include "CommentsToFeatures.h"
#include "MatrixBlockPartition.h"

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

#include "FFPageIndexer.h"
#include "AbstractIndexer.h"

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
    cout << "Usage: commentsSetName enablePartition blockDimensionX "
            "blockDimensionY batchSize path/to/weights/and/bias(leave empty if "
            "generate random)"
         << endl;
    exit(-1);
  }

  bool enablePartition = false;

  long totalTime = 0;

  commentsSetName = std::string(argv[1]);
  if (strcmp(argv[2], "Y") == 0) {
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

  ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
  ff::loadLibrary(pdbClient, "libraries/libFFPageIndexer.so");

  ff::createDatabase(pdbClient, db);

  if (!pdbClient.createSet<FFMatrixBlock>(
          db, "w1", errMsg, (size_t)64 * (size_t)1024 * (size_t)1024,
          "W1", nullptr, nullptr, true)) {
    cout << "Not able to create set: " + errMsg;
    //exit(-1); //It is possible that the set exists
  } else {
    cout << "Created set.\n";
  }

  if (!pdbClient.createSet<FFMatrixBlock>(
          db, "w2", errMsg, (size_t)64 * (size_t)1024 * (size_t)1024,
          "W1", nullptr, nullptr, true)) {
    cout << "Not able to create set: " + errMsg;
    //exit(-1); //It is possible that the set exists
  } else {
    cout << "Created set.\n";
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    pdb::Handle<pdb::AbstractIndexer> indexer = makeObject<FFPageIndexer>(10, 3);

    if (!pdbClient.addTypeIndexer<FFMatrixBlock>(db, indexer)) {
      cout << "Not able to create set: " + errMsg;
      //exit(-1); //It is possible that the set exists
    } else {
      cout << "Created set.\n";
    } 

  }

  bool block_padding = true;
  block_x = 10;
  block_y = 10;

  ff::load_matrix_data(pdbClient, "/usr/data/shared_pages_test/w1.np", db, "w1", block_x, block_y, !block_padding, !block_padding, errMsg);
  ff::load_matrix_data(pdbClient, "/usr/data/shared_pages_test/w2.np", db, "w2", block_x, block_y, !block_padding, !block_padding, errMsg);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};
    // ff::print(pdbClient, db, "w1");
    // ff::print(pdbClient, db, "w2");
    ff::print_stats(pdbClient, db, "w1");
    ff::print_stats(pdbClient, db, "w2");
  }

  return 0;
}