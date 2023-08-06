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
  int numFeatures, numNeurons, numLabels;
  if (argc < 3) {
    cout << "Usage: blockDimensionX blockDimensionY batchSize numFeatures numNeurons numLabels reloading(Y/N)"
            " path/to/weights/and/bias(leave empty if generate random)"
         << endl;
    exit(-1);
  }

  block_x = atoi(argv[1]);
  block_y = atoi(argv[2]);
  batch_size = atoi(argv[3]);
  numFeatures = atoi(argv[4]);
  numNeurons = atoi(argv[5]);
  numLabels = atoi(argv[6]);

  if (argc >= 7) {
      if (strcmp(argv[7], "N")==0) {
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

      ff::createSet(pdbClient, "ff", "inputs", "inputs", 4);
      ff::createSet(pdbClient, "ff", "label", "label", 4);

      ff::createSet(pdbClient, "ff", "w1", "W1", 64);
      ff::createSet(pdbClient, "ff", "b1", "B1", 4);

      ff::createSet(pdbClient, "ff", "wo", "WO", 64);
      ff::createSet(pdbClient, "ff", "bo", "BO", 4);

  }

  ff::createSet(pdbClient, "ff", "output", "Output", 64);

  ff::createSet(pdbClient, "ff", "y1", "Y1", 4);


  ff::createSet(pdbClient, "ff", "yo", "YO", 4);

  if (!generate && reloadData) {
    input_path = string(argv[4]) + "/input.out";
    labels_path = string(argv[4]) + "/labels.out";
    w1_path = string(argv[4]) + "/w1.out";
    wo_path = string(argv[4]) + "/wo.out";
    b1_path = string(argv[4]) + "/b1.out";
    bo_path = string(argv[4]) + "/bo.out";

    // load the input data
    ff::load_matrix_data(pdbClient, input_path, "ff", "inputs",
                                      block_x, block_y, false, false, errMsg);
    sleep(20);
    (void)ff::load_matrix_data(pdbClient, w1_path, "ff", "w1", block_x, block_y,
                               false, false, errMsg);
    sleep(20);
    (void)ff::load_matrix_data(pdbClient, wo_path, "ff", "wo", block_x, block_y,
                               false, false, errMsg);
    (void)ff::load_matrix_data(pdbClient, b1_path, "ff", "b1", block_x, block_y,
                               false, true, errMsg);
    (void)ff::load_matrix_data(pdbClient, bo_path, "ff", "bo", block_x, block_y,
                               false, true, errMsg);
  } else if (reloadData) {

    std::cout << "To load matrix for ff:inputs" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "inputs", batch_size, numFeatures, block_x,
                   block_y, false, false, errMsg);

    std::cout << "To load matrix for ff:w1" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "w1", numNeurons, numFeatures, block_x, block_y,
                   false, false, errMsg);

    std::cout << "To load matrix for ff:b1" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "b1", numNeurons, 1, block_x,
                   1, false, true, errMsg);

    std::cout << "To load matrix for ff:wo" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "wo", numLabels, numNeurons, block_x,
                   block_x, false, false, errMsg);
    // 2 x 1
    std::cout << "To load matrix for ff:bo" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "bo", numLabels, 1, block_x, 1,
                   false, true, errMsg);
  }

  float dropout_rate = 0.5;



  auto begin = std::chrono::high_resolution_clock::now();

  ff::inference_unit(pdbClient, "ff", "w1", "wo", "inputs", "b1", "bo",
                "output", dropout_rate);

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "*****FFTest End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;



  vector<vector<float>> labels_test;

  if (!generate)
    ff::load_matrix_from_file(labels_path, labels_test);

  int count = 0;
  int correct = 0;
  {
    pdb::makeObjectAllocatorBlock(1024 * 1024 * 256, true);

    auto it = pdbClient.getSetIterator<FFMatrixBlock>("ff", "output");


    for (auto r : it) {

      count++;

      float *data = r->getRawDataHandle()->c_ptr();
      int i = 0;
      int j = r->getBlockRowIndex() * r->getRowNums();
      while (i < r->getRowNums() * r->getColNums()) {
        if (!generate && j >= labels_test.size())
          break;

        cout << data[i] << ", " << data[i + 1] << endl;

        if (!generate) {
          int pos1 = data[i] > data[i + 1] ? 0 : 1;
          int pos2 = labels_test[j][0] > labels_test[j][1] ? 0 : 1;

          if (pos1 == pos2)
            correct++;
        }

        i += r->getColNums();
        j++;
      }
    }

    if (!generate)
      cout << "Accuracy: " << correct << "/" << labels_test.size() << std::endl;
  }

  std::cout << "count=" << count << std::endl;

  sleep(20);

  return 0;
}
