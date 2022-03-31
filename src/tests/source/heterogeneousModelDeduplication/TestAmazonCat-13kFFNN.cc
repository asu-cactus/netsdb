#ifndef TEST_AMAZONCAT_13K_CC
#define TEST_AMAZONCAT_13K_CC

#include "PDBString.h"
#include "PDBMap.h"
#include "DataTypes.h"
#include "TensorBlockIndex.h"
#include "InterfaceFunctions.h"
#include "PDBClient.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SimpleFF.h"
#include "FFMatrixBlockScanner.h"
#include "FFMatrixWriter.h"
#include "FFAggMatrix.h"
#include "FFTransposeMult.h"
#include "SemanticClassifier.h"
#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cmath>
#include <cstdlib>

using namespace pdb;

int main(int argc, char *argv[]) {

	makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

	bool reloadData = true;
	string errMsg;
	string input_path, labels_path, w1_path, wo_path, b1_path, bo_path;
	int block_x, block_y, batch_size;
	int numFeatures, numNeurons, numLabels;
	if (argc < 3) {
		cout << "Usage: blockDimensionX blockDimensionY batchSize Y"
            "path/to/weights/and/bias(leave empty if generate random)"
            << endl;
        exit(-1);
    }

  block_x = atoi(argv[1]);
  block_y = atoi(argv[2]);
  batch_size = atoi(argv[3]);
  numFeatures = 203882;
  numNeurons = 1000;
  numLabels = 1330;

  if (argc >= 4) {
      if (strcmp(argv[4], "N")==0) {
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

  ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixPartitioner.so");

  if (reloadData) {

      ff::createDatabase(pdbClient, "ff");
      ff::setup(pdbClient, "ff");

      ff::createSet(pdbClient, "ff", "inputs", "inputs", 64);
      ff::createSet(pdbClient, "ff", "label", "label", 64);

      ff::createSet(pdbClient, "ff", "w1", "W1", 64);
      ff::createSet(pdbClient, "ff", "b1", "B1", 64);

      ff::createSet(pdbClient, "ff", "wo", "WO", 64);
      ff::createSet(pdbClient, "ff", "bo", "BO", 64);

  }

  ff::createSet(pdbClient, "ff", "output", "Output", 256);
  ff::createSet(pdbClient, "ff", "y1", "Y1", 64);
  ff::createSet(pdbClient, "ff", "yo", "YO", 64);

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
    (void)ff::load_matrix_data(pdbClient, w1_path, "ff", "w1", block_x, block_y,
                               false, false, errMsg);
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
                   block_y, false, true, errMsg);

    std::cout << "To load matrix for ff:wo" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "wo", numLabels, numNeurons, block_x,
                   block_y, false, false, errMsg);

    std::cout << "To load matrix for ff:bo" << std::endl;
    ff::loadMatrix(pdbClient, "ff", "bo", numLabels, 1, block_x, block_y,
                   false, true, errMsg);
  }

  double dropout_rate = 0.0;

  auto begin = std::chrono::high_resolution_clock::now();

  ff::inference_unit(pdbClient, "ff", "w1", "wo", "inputs", "b1", "bo",
                "output", dropout_rate);

  auto end = std::chrono::high_resolution_clock::now();
  std::cout << "*****FFTest End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  return 0;
}

#endif