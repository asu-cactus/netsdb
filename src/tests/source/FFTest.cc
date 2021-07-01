#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "PDBClient.h"

#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"

using namespace std;

int main(int argc, char *argv[]) {

  bool reloadData = true;
  string errMsg;
  string input_path, labels_path, w1_path, w2_path, wo_path, b1_path, b2_path,
      bo_path;
  int block_x, block_y, batch_size;

  if (argc < 3) {
    cout << "Usage: blockDimensionX blockDimensionY Y"
            "path/to/weights/and/bias(leave empty if generate random)"
         << endl;
    exit(-1);
  }

  block_x = atoi(argv[1]);
  block_y = atoi(argv[2]);

  if (argc >= 4) {
      if (strcmp(argv[3], "N")==0) {
          reloadData = false;
          std::cout << "WARNING: we do not reload data!" << std::endl;
      }
  }
  cout << "Using block dimensions " << block_x << ", " << block_y << endl;

  bool generate = true;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("FFclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);



  if (reloadData) {

      ff::createDatabase(pdbClient, "ff");
      ff::setup(pdbClient, "ff");

      ff::createSet(pdbClient, "ff", "inputs", "inputs", 64);
      ff::createSet(pdbClient, "ff", "label", "label", 64);

      ff::createSet(pdbClient, "ff", "w1", "W1", 64);
      ff::createSet(pdbClient, "ff", "b1", "B1", 64);

      ff::createSet(pdbClient, "ff", "w2", "W2", 64);
      ff::createSet(pdbClient, "ff", "b2", "B2", 64);

      ff::createSet(pdbClient, "ff", "wo", "WO", 64);
      ff::createSet(pdbClient, "ff", "bo", "BO", 64);

  }

  ff::createSet(pdbClient, "ff", "output", "Output", 64);

  ff::createSet(pdbClient, "ff", "y1", "Y1", 64);

  ff::createSet(pdbClient, "ff", "y2", "Y2", 64);

  ff::createSet(pdbClient, "ff", "yo", "YO", 64);

  if (!generate && reloadData) {
    input_path = string(argv[4]) + "/input.out";
    labels_path = string(argv[4]) + "/labels.out";
    w1_path = string(argv[4]) + "/w1.out";
    w2_path = string(argv[4]) + "/w2.out";
    wo_path = string(argv[4]) + "/wo.out";
    b1_path = string(argv[4]) + "/b1.out";
    b2_path = string(argv[4]) + "/b2.out";
    bo_path = string(argv[4]) + "/bo.out";

    // load the input data
    ff::load_matrix_data(pdbClient, input_path, "ff", "inputs",
                                      block_x, block_y, false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, w1_path, "ff", "w1", block_x, block_y,
                               false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, w2_path, "ff", "w2", block_x, block_y,
                               false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, wo_path, "ff", "wo", block_x, block_y,
                               false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, b1_path, "ff", "b1", block_x, block_y,
                               false, true, errMsg);
    (void)ff::load_matrix_data(pdbClient, b2_path, "ff", "b2", block_x, block_y,
                               false, true, errMsg);
    (void)ff::load_matrix_data(pdbClient, bo_path, "ff", "bo", block_x, block_y,
                               false, true, errMsg);
  } else if (reloadData) {
    batch_size = 100;
    // block_x = 20;
    // block_y = 20;

    int features = 100;
    int hid1_size = 128;
    int hid2_size = 256;
    int num_labels = 2;

    // X x features_size = None x 5000
    std::cout << "To load matrix for ff:inputs" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "inputs", batch_size, features, block_x,
                   block_y, false, false, errMsg);
    // X x labels_size = ???
    // ff::loadMatrix(pdbClient, "ff", "label", 64, null, 1, 100, 2, errMsg);

    // 128 x features_size = 128 x 5000
    std::cout << "To load matrix for ff:w1" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "w1", hid1_size, features, block_x, block_y,
                   false, false, errMsg);
    // 128 x 1
    std::cout << "To load matrix for ff:b1" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "b1", hid1_size, 1, block_x,
                   block_y, false, true, errMsg);

    // 256 x 128
    std::cout << "To load matrix for ff:w2" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "w2", hid2_size, hid1_size, block_x,
                   block_y, false, false, errMsg);
    // 256 x 1
    std::cout << "To load matrix for ff:b2" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "b2", hid2_size, 1, block_x, block_y, false,
                   true, errMsg);

    // 2 x 256
    std::cout << "To load matrix for ff:wo" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "wo", num_labels, hid2_size, block_x,
                   block_y, false, false, errMsg);
    // 2 x 1
    std::cout << "To load matrix for ff:bo" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "bo", num_labels, 1, block_x, block_y,
                   false, true, errMsg);
  }

  double dropout_rate = 0.5;



  auto begin = std::chrono::high_resolution_clock::now();

  ff::inference(pdbClient, "ff", "w1", "w2", "wo", "inputs", "b1", "b2", "bo",
                "output", dropout_rate);

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "*****FFTest End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;



  vector<vector<double>> labels_test;

  if (!generate)
    ff::load_matrix_from_file(labels_path, labels_test);

  int correct = 0;
  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    auto it = pdbClient.getSetIterator<FFMatrixBlock>("ff", "output");

    for (auto r : it) {
      double *data = r->getRawDataHandle()->c_ptr();
      int i = 0;
      int j = r->getBlockRowIndex() * r->getRowNums();
      while (i < r->getRowNums() * r->getColNums()) {
        if (!generate && j >= labels_test.size())
          break;
        // double a = exp(data[i]);
        // double b = exp(data[i + 1]);
        // double sum = a + b;

        cout << data[i] << ", " << data[i + 1] << endl;

        if (!generate) {
          int pos1 = data[i] > data[i + 1] ? 0 : 1;
          int pos2 = labels_test[j][0] > labels_test[j][1] ? 0 : 1;

          if (pos1 == pos2)
            correct++;
        }

        // cout << (a/sum) << ", " << (b/sum) << " : " << labels_test[j][0] <<
        // ", " << labels_test[j][1] << endl;
        i += r->getColNums();
        j++;
      }
    }

    if (!generate)
      cout << "Accuracy: " << correct << "/" << labels_test.size() << std::endl;
  }

  sleep(20);

  return 0;
}
