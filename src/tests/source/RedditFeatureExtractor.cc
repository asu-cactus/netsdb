#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "FFMatrixBlock.h"
#include "FFMatrixBlockScanner.h"
#include "RedditComment.h"
#include "RedditCommentLabelJoin.h"
#include <ScanUserSet.h>
#include <WriteUserSet.h>

#include "FFMatrixUtil.h"
#include "PDBClient.h"
#include "SimpleFF.h"

#include <chrono>
#include <ctime>

using namespace std;
using namespace std::chrono;

void getCommentFeatures(pdb::Handle<reddit::Comment> &comment,
                        vector<double> &features) {
  features[0] = ((double)comment->archived);

  system_clock::time_point tp_epoch;
  time_point<system_clock, duration<int>> tp_seconds(
      duration<int>(comment->author_created_utc));
  system_clock::time_point tp(tp_seconds);

  time_t tt = system_clock::to_time_t(tp);
  tm utc_tm = *gmtime(&tt);

  features[1] = (double)utc_tm.tm_mday / 31.0;
  features[2] = ((double)comment->author_patreon_flair);
  features[3] = ((double)comment->can_gild);
  features[4] = ((double)comment->can_mod_post);
  features[5] = ((double)comment->collapsed);
  features[6] = ((double)comment->controversiality);

  time_point<system_clock, duration<int>> tq_seconds(
      duration<int>(comment->created_utc));
  system_clock::time_point tq(tq_seconds);

  time_t ttq = system_clock::to_time_t(tq);
  tm utc_tq = *gmtime(&ttq);

  features[7] = (double)utc_tq.tm_mday / 31.0;
  features[8] = ((double)comment->gilded);
  // features[9] = ((double)comment->is_submitter);
  // features[10] = ((double)comment->no_follows);
  // features[11] = ((double)comment->retrieved_on);
  // features[12] = ((double)comment->score);
  // features[13] = ((double)comment->send_replies);
  // features[14] = ((double)comment->stickied);
}

void extract_features(pdb::PDBClient &pdbClient, int block_x, int block_y,
                      int total_features, int batch_size, string db,
                      string set) {
  string errMsg;

  ff::createSet(pdbClient, db, set, set);
  pdb::makeObjectAllocatorBlock(128 * 1024 * 1024, true);

  // int numXBlocks = ceil(totalX / (double)blockX); // no idea how many
  // comments
  int numYBlocks = ceil(total_features / (double)block_y);

  // will contain comment features
  vector<double> features;
  features.resize(total_features);

  auto it = pdbClient.getSetIterator<reddit::Comment>(db, "comments");
  int pos = 0;       // total comments seen
  int block_pos = 0; // total X blocks seen

  pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> storeMatrix1 =
      pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

  int total_blocks = 0;
  int batch_pos = 0;

  for (auto r : it) {

    // Create new row of blocks
    if (batch_pos == 0) {
      int new_blocks_pushed = 0;
      try {
        for (; new_blocks_pushed < numYBlocks; new_blocks_pushed++) {
          pdb::Handle<FFMatrixBlock> myData = pdb::makeObject<FFMatrixBlock>(
              block_pos, new_blocks_pushed, block_x, block_y, batch_size,
              total_features);
          storeMatrix1->push_back(myData);
        }
      } catch (pdb::NotEnoughSpace &n) {
        cout << "Not enough space. Sending existing blocks to db to make space."
             << endl;
        // Remove the new blocks created
        for (int i = 0; i < new_blocks_pushed; i++) {
          storeMatrix1->pop_back();
        }

        if (!pdbClient.sendData<FFMatrixBlock>(
                std::pair<std::string, std::string>(set, db), storeMatrix1,
                errMsg)) {
          cout << "Failed to send data to dispatcher server" << endl;
          exit(1);
        }

        cout << "Sent " << storeMatrix1->size() << " blocks to db" << endl;
        pdb::makeObjectAllocatorBlock(128 * 1024 * 1024, true);
        storeMatrix1 =
            pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

        new_blocks_pushed = 0;
        for (; new_blocks_pushed < numYBlocks; new_blocks_pushed++) {
          pdb::Handle<FFMatrixBlock> myData = pdb::makeObject<FFMatrixBlock>(
              block_pos, new_blocks_pushed, block_x, block_y, -1,
              total_features);
          storeMatrix1->push_back(myData);
        }
      }

      total_blocks += new_blocks_pushed;
    }

    getCommentFeatures(r, features);

    int start = storeMatrix1->size() - numYBlocks;
    for (int i = 0; i < numYBlocks; i++) {
      for (int j = 0; j < block_y; j++) {
        int curY = (i * block_y + j);
        double data = curY >= total_features
                          ? 0.0 // padding to adjust to block dimensions
                          : features[curY];
        (*((*storeMatrix1)[start + i]
               ->getRawDataHandle()))[batch_pos * block_y + j] = data;
      }
    }

    batch_pos++;
    if (batch_pos == block_x) {
      batch_pos = 0;
      block_pos++;
    }

    pos++;
  }

  // padding required for the last row of blocks
  if (batch_pos != 0) {
    // row padding to keep block_x X block_y dimensions for each block
    int start = storeMatrix1->size() - numYBlocks;
    for (int b = 0; b < numYBlocks; b++) {
      for (int i = batch_pos; i < block_x; i++) {
        for (int j = 0; j < block_y; j++) {
          (*((*storeMatrix1)[start + b]->getRawDataHandle()))[i * block_y + j] =
              0.0;
        }
      }
    }
  }

  // Send the remaining blocks to database
  if (!pdbClient.sendData<FFMatrixBlock>(
          std::pair<std::string, std::string>(set, db), storeMatrix1, errMsg)) {
    cout << "Failed to send data to dispatcher server" << endl;
    exit(1);
  }

  pdbClient.flushData(errMsg);

  cout << "Sent " << storeMatrix1->size() << " blocks to db" << endl;
}

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

  int total_features = 9; // Change this later

  string db = "redditDB", set = "comment_features";

  ff::setup(pdbClient, db);

  ff::createSet(pdbClient, db, "w1", "W1");
  ff::createSet(pdbClient, db, "b1", "B1");

  ff::createSet(pdbClient, db, "w2", "W2");
  ff::createSet(pdbClient, db, "b2", "B2");

  ff::createSet(pdbClient, db, "wo", "WO");
  ff::createSet(pdbClient, db, "bo", "BO");

  ff::createSet(pdbClient, db, "output", "Output");

  ff::createSet(pdbClient, db, "result", "Result");

  string main_path = string(argv[4]);
  string input_path = main_path + "/input.out";
  string w1_path = main_path + "/w1.out";
  string w2_path = main_path + "/w2.out";
  string wo_path = main_path + "/wo.out";
  string b1_path = main_path + "/b1.out";
  string b2_path = main_path + "/b2.out";
  string bo_path = main_path + "/bo.out";

  if (!generate) {
    // load the input data
    (void)ff::load_matrix_data(pdbClient, w1_path, db, "w1", block_x, block_y,
                               -1, errMsg);
    (void)ff::load_matrix_data(pdbClient, w2_path, db, "w2", block_x, block_y,
                               -1, errMsg);
    (void)ff::load_matrix_data(pdbClient, wo_path, db, "wo", block_x, block_y,
                               -1, errMsg);
    (void)ff::load_matrix_data(pdbClient, b1_path, db, "b1", block_x, block_y,
                               batch_size, errMsg);
    (void)ff::load_matrix_data(pdbClient, b2_path, db, "b2", block_x, block_y,
                               batch_size, errMsg);
    (void)ff::load_matrix_data(pdbClient, bo_path, db, "bo", block_x, block_y,
                               batch_size, errMsg);
  } else {
    int hid1_size = 128;
    int hid2_size = 256;
    int num_labels = 2;

    // X x features_size = None x 5000
    // ff::loadMatrix(pdbClient, db, "inputs", batch_size, features, block_x,
    //             block_y, errMsg);
    // X x labels_size = ???
    // ff::loadMatrix(pdbClient, db, "label", 64, null, 1, 100, 2, errMsg);

    // 128 x features_size = 128 x 5000
    ff::loadMatrix(pdbClient, db, "w1", hid1_size, total_features, block_x,
                   block_y, errMsg);
    // 128 x 1
    ff::loadMatrix(pdbClient, db, "b1", hid1_size, batch_size, block_x, block_y,
                   errMsg);

    // 256 x 128
    ff::loadMatrix(pdbClient, db, "w2", hid2_size, hid1_size, block_x, block_y,
                   errMsg);
    // 256 x 1
    ff::loadMatrix(pdbClient, db, "b2", hid2_size, batch_size, block_x, block_y,
                   errMsg);

    // 2 x 256
    ff::loadMatrix(pdbClient, db, "wo", num_labels, hid2_size, block_x, block_y,
                   errMsg);
    // 2 x 1
    ff::loadMatrix(pdbClient, db, "bo", num_labels, batch_size, block_x,
                   block_y, errMsg);
  }

  extract_features(pdbClient, block_x, block_y, total_features, batch_size, db,
                   set);

  double dropout_rate = 0.5;

  ff::inference(pdbClient, db, "w1", "w2", "wo", set, "b1", "b2", "bo",
                "output", dropout_rate);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    auto it = pdbClient.getSetIterator<FFMatrixBlock>(db, "output");

    for (auto r : it) {
      double *data = r->getRawDataHandle()->c_ptr();
      int i = 0;
      int j = r->getBlockRowIndex() * r->getRowNums();
      while (i < r->getRowNums() * r->getColNums()) {
        cout << data[i] << ", " << data[i + 1] << endl;
        i += r->getColNums();
        j++;
      }
    }
  }

  ff::loadLibrary(pdbClient, "libraries/libRedditCommentLabelJoin.so");

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>(db, "output");

    pdb::Handle<pdb::Computation> readB =
        makeObject<ScanUserSet<reddit::Comment>>(db, "comments");

    pdb::Handle<pdb::Computation> join =
        pdb::makeObject<reddit::CommentLabelJoin>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the writer
    pdb::Handle<pdb::Computation> sumWriter =
        pdb::makeObject<WriteUserSet<reddit::Comment>>(db, "result");
    sumWriter->setInput(join);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, sumWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      exit(1);
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    auto it = pdbClient.getSetIterator<reddit::Comment>(db, "result");

    for (auto r : it) {
      cout << r->label << endl;
    }
  }

  return 0;
}
