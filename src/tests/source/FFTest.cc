#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "FFAggMatrix.h"
#include "FFInputLayerJoin.h"
#include "FFMatrixBlock.h"
#include "FFMatrixBlockScanner.h"
#include "FFMatrixData.h"
#include "FFMatrixMeta.h"
#include "FFMatrixWriter.h"
#include "FFReluBiasSum.h"
#include "FFTransposeBiasSum.h"
#include "FFTransposeMult.h"
#include "FFRowAggregate.h"
#include "FFOutputLayer.h"

#include "PDBClient.h"

using namespace std;

void load_matrix_from_file(string path, vector<vector<double>> &matrix) {
  if (path.size() == 0) {
    throw runtime_error("Invalid filepath: " + path);
  }

  int totalX, totalY;

  /// 1. Load the data from the file

  // open the input file
  ifstream is(path);
  while (is.peek() == '#' || is.peek() == ' ')
    is.ignore();

  // load the data stats
  is >> totalX;

  while (is.peek() == ',' || is.peek() == ' ')
    is.ignore();

  is >> totalY;

  cout << totalX << ", " << totalY << endl;

  double val;
  for (int i = 0; i < totalX; i++) {
    vector<double> line;
    for (int j = 0; j < totalY; j++) {
      is >> val;
      line.push_back(val);
      while (is.peek() == ',' || is.peek() == ' ')
        is.ignore();
    }
    matrix.push_back(line);
  }
}

int load_matrix_data(pdb::PDBClient &pdbClient, string path, String dbName,
                     String setName, int blockX, int blockY, int duplicateY,
                     string &errMsg) {
  if (path.size() == 0) {
    throw runtime_error("Invalid filepath: " + path);
  }

  int totalX, totalY;

  /// 1. Load the data from the file

  // open the input file
  ifstream is(path);
  while (is.peek() == '#' || is.peek() == ' ')
    is.ignore();

  // load the data stats
  is >> totalX;

  while (is.peek() == ',' || is.peek() == ' ')
    is.ignore();

  is >> totalY;

  cout << totalX << ", " << totalY << endl;

  vector<vector<double>> matrix;

  double val;
  for (int i = 0; i < totalX; i++) {
    vector<double> line;
    for (int j = 0; j < totalY; j++) {
      is >> val;
      line.push_back(val);
      while (is.peek() == ',' || is.peek() == ' ')
        is.ignore();
    }
    matrix.push_back(line);
  }

  if (matrix.size() == 0) {
    throw runtime_error("Invalid matrix file: " + path);
  }

  int total = 0;
  pdb::makeObjectAllocatorBlock(128 * 1024 * 1024, true);

  pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> storeMatrix1 =
      pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

  int numXBlocks = ceil(totalX / (double)blockX);
  int numYBlocks;
  if (duplicateY != -1 && duplicateY > totalY) {
    if (totalY != 1) {
      cout << "Cannot duplicate if the original matrix has more than 1 column"
           << endl;
      exit(1);
    }
    numYBlocks = ceil(duplicateY / (double)blockY);
  } else {
    numYBlocks = ceil(totalY / (double)blockY);
  }

  if (duplicateY == -1)
    duplicateY = totalY;

  try {
    for (int i = 0; i < numXBlocks; i++) {
      for (int j = 0; j < numYBlocks; j++) {
        pdb::Handle<FFMatrixBlock> myData =
            pdb::makeObject<FFMatrixBlock>(i, j, blockX, blockY, matrix.size(), matrix[0].size());

        for (int ii = 0; ii < blockX; ii++) {
          for (int jj = 0; jj < blockY; jj++) {
            int curX = (i * blockX + ii);
            int curY = (j * blockY + jj);

            double data = curX >= totalX || curY >= duplicateY
                              ? 0 // padding to adjust to block dimensions
                              : curY > matrix[curX].size() ? matrix[curX][0]
                                                           : matrix[curX][curY];
            (*(myData->getRawDataHandle()))[ii * blockY + jj] = data;
          }
        }

        // cout << "New block: " << total << endl;
        storeMatrix1->push_back(myData);
        total++;
      }
    }
    if (!pdbClient.sendData<FFMatrixBlock>(
            pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
      cout << "Failed to send data to dispatcher server" << endl;
      exit(1);
    }
  } catch (NotEnoughSpace &e) {
    cout << "Failed to send data to dispatcher server" << endl;
    exit(1);
  }
  cout << setName << "(" << totalX << "x" << totalY << "): (" << numXBlocks
       << " x " << numYBlocks << ")" << total << " blocks = " << blockX << " x "
       << blockY << " each" << endl;

  // to write back all buffered records
  pdbClient.flushData(errMsg);

  return matrix.size();
}

void print(pdb::PDBClient &pdbClient, string dbName, string setName) {
  auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);

  for (auto r : it) {
    double *data = r->getRawDataHandle()->c_ptr();
    for (int i = 0; i < r->getRowNums() * r->getColNums(); i++) {
      cout << data[i] << ",";
    }
    cout << endl;
  }
}

void print_stats(pdb::PDBClient &pdbClient, string dbName, string setName) {
  int rows = 0, cols = 0, blocks = 0;
  int totalRows = 0, totalCols = 0;
  int blockRows = 0, blockCols = 0;
  auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);

  for (auto r : it) {
    cout << r->getBlockRowIndex() << "," << r->getBlockColIndex() << ";";
    rows = r->getRowNums();
    cols = r->getColNums();
    if (r->getBlockRowIndex() == 0) {
      totalRows += r->getRowNums();
      blockRows += 1;
    }
    if (r->getBlockColIndex() == 0) {
      totalCols += r->getColNums();
      blockCols += 1;
    }
    blocks++;
  }

  cout << "\n"
       << setName << " (" << blockRows << " X " << blockCols << ") (" << blocks
       << ") : (" << totalRows << " x " << totalCols
       << "), Each block size: " << rows << " x " << cols << endl;
}

void loadMatrix(pdb::PDBClient &pdbClient, String dbName, String setName,
                int totalX, int totalY, int blockX, int blockY,
                string &errMsg) {

  int total = 0;
  pdb::makeObjectAllocatorBlock(128 * 1024 * 1024, true);

  pdb::Handle<pdb::Vector<pdb::Handle<FFMatrixBlock>>> storeMatrix1 =
      pdb::makeObject<pdb::Vector<pdb::Handle<FFMatrixBlock>>>();

  int numXBlocks = ceil(totalX / (double)blockX);
  int numYBlocks = ceil(totalY / (double)blockY);

  try {
    for (int i = 0; i < numXBlocks; i++) {
      for (int j = 0; j < numYBlocks; j++) {
        pdb::Handle<FFMatrixBlock> myData =
            pdb::makeObject<FFMatrixBlock>(i, j, blockX, blockY);

        for (int ii = 0; ii < blockX; ii++) {
          for (int jj = 0; jj < blockY; jj++) {
            // row = i * blockX + ii, col = j * blockY + jj
            double data =
                (i * blockX + ii) >= totalX || (j * blockY + jj) >= totalY
                    ? 0
                    : i + j + ii + jj;
            (*(myData->getRawDataHandle()))[ii * blockY + jj] = data;
          }
        }

        // cout << "New block: " << total << endl;
        storeMatrix1->push_back(myData);
        total++;
      }
    }
    if (!pdbClient.sendData<FFMatrixBlock>(
            pair<string, string>(setName, dbName), storeMatrix1, errMsg)) {
      cout << "Failed to send data to dispatcher server" << endl;
      exit(1);
    }
  } catch (NotEnoughSpace &e) {
    cout << "Failed to send data to dispatcher server" << endl;
    exit(1);
  }
  cout << setName << "(" << totalX << "x" << totalY << "): (" << numXBlocks
       << " x " << numYBlocks << ")" << total << " blocks = " << blockX << " x "
       << blockY << " each" << endl;

  // to write back all buffered records
  pdbClient.flushData(errMsg);
}

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
  if (!pdbClient.createSet<FFMatrixBlock>(
          dbName, setName, errMsg, (size_t)64 * (size_t)1024 * (size_t)1024,
          setName1)) {
    cout << "Not able to create set: " + errMsg;
    exit(-1);
  } else {
    cout << "Created set.\n";
  }
}

void createDatabase(pdb::PDBClient &pdbClient, string dbName) {
  string errMsg;
  if (!pdbClient.createDatabase(dbName, errMsg)) {
    cout << "Not able to create database: " << errMsg << endl;
    exit(-1);
  } else {
    cout << "Created database" << endl;
  }
}

int main(int argc, char *argv[]) {
  string errMsg;
  string input_path, labels_path, w1_path, w2_path, wo_path, b1_path, b2_path,
      bo_path;
  int block_x, block_y, batch_size;

  if (argc < 2) {
    cout << "Usage: blockDimensionX blockDimensionY "
            "path/to/weights/and/bias(leave empty if generate random)"
         << endl;
    exit(-1);
  }

  block_x = atoi(argv[1]);
  block_y = atoi(argv[2]);
  cout << "Using block dimensions " << block_x << ", " << block_y << endl;

  bool generate = argc == 3;

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("FFclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

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

  createDatabase(pdbClient, "ff");

  createSet(pdbClient, "ff", "inputs", "inputs");
  createSet(pdbClient, "ff", "label", "label");

  createSet(pdbClient, "ff", "w1", "W1");
  createSet(pdbClient, "ff", "b1", "B1");
  createSet(pdbClient, "ff", "temp_y1", "TempY1");
  createSet(pdbClient, "ff", "y1", "Y1");

  createSet(pdbClient, "ff", "w2", "W2");
  createSet(pdbClient, "ff", "b2", "B2");
  createSet(pdbClient, "ff", "temp_y2", "TempY2");
  createSet(pdbClient, "ff", "y2", "Y2");

  createSet(pdbClient, "ff", "wo", "WO");
  createSet(pdbClient, "ff", "bo", "BO");
  createSet(pdbClient, "ff", "temp_yo", "TempYO");
  createSet(pdbClient, "ff", "yo", "YO");

  createSet(pdbClient, "ff", "yo_exp_sum", "YOExpSum");
  createSet(pdbClient, "ff", "output", "Output");

  if (!generate) {
    input_path = string(argv[3]) + "/input.out";
    labels_path = string(argv[3]) + "/labels.out";
    w1_path = string(argv[3]) + "/w1.out";
    w2_path = string(argv[3]) + "/w2.out";
    wo_path = string(argv[3]) + "/wo.out";
    b1_path = string(argv[3]) + "/b1.out";
    b2_path = string(argv[3]) + "/b2.out";
    bo_path = string(argv[3]) + "/bo.out";

    // load the input data
    batch_size = load_matrix_data(pdbClient, input_path, "ff", "inputs",
                                  block_x, block_y, -1, errMsg);
    (void)load_matrix_data(pdbClient, w1_path, "ff", "w1", block_x, block_y, -1,
                           errMsg);
    (void)load_matrix_data(pdbClient, w2_path, "ff", "w2", block_x, block_y, -1,
                           errMsg);
    (void)load_matrix_data(pdbClient, wo_path, "ff", "wo", block_x, block_y, -1,
                           errMsg);
    (void)load_matrix_data(pdbClient, b1_path, "ff", "b1", block_x, block_y,
                           batch_size, errMsg);
    (void)load_matrix_data(pdbClient, b2_path, "ff", "b2", block_x, block_y,
                           batch_size, errMsg);
    (void)load_matrix_data(pdbClient, bo_path, "ff", "bo", block_x, block_y,
                           batch_size, errMsg);
  } else {
    batch_size = 400;
    // block_x = 20;
    // block_y = 20;

    int features = 5000;
    int hid1_size = 128;
    int hid2_size = 256;
    int num_labels = 2;

    // X x features_size = None x 5000
    loadMatrix(pdbClient, "ff", "inputs", batch_size, features, block_x,
               block_y, errMsg);
    // X x labels_size = ???
    // loadMatrix(pdbClient, "ff", "label", 64, null, 1, 100, 2, errMsg);

    // 128 x features_size = 128 x 5000
    loadMatrix(pdbClient, "ff", "w1", hid1_size, features, block_x, block_y,
               errMsg);
    // 128 x 1
    loadMatrix(pdbClient, "ff", "b1", hid1_size, batch_size, block_x, block_y,
               errMsg);

    // 256 x 128
    loadMatrix(pdbClient, "ff", "w2", hid2_size, hid1_size, block_x, block_y,
               errMsg);
    // 256 x 1
    loadMatrix(pdbClient, "ff", "b2", hid2_size, batch_size, block_x, block_y,
               errMsg);

    // 2 x 256
    loadMatrix(pdbClient, "ff", "wo", num_labels, hid2_size, block_x, block_y,
               errMsg);
    // 2 x 1
    loadMatrix(pdbClient, "ff", "bo", num_labels, batch_size, block_x, block_y,
               errMsg);
  }

  double dropout_rate = 0.5;

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "w1");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("ff", "inputs");

    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFTransposeMult>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation =
        pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>("ff", "temp_y1");
    myWriter->setInput(myAggregation);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "temp_y1");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("ff", "b1");

    pdb::Handle<pdb::Computation> reluBias =
        pdb::makeObject<FFReluBiasSum>(dropout_rate);
    reluBias->setInput(0, readA);
    reluBias->setInput(1, readB);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>("ff", "y1");
    myWriter->setInput(reluBias);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "ff", "w1");
    print_stats(pdbClient, "ff", "inputs");
    print_stats(pdbClient, "ff", "b1");
    print_stats(pdbClient, "ff", "temp_y1");
    print_stats(pdbClient, "ff", "y1");
  }

  pdbClient.deleteSet("ff", "temp_y1");

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "w2");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("ff", "y1");

    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFInputLayerJoin>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation =
        pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>("ff", "temp_y2");
    myWriter->setInput(myAggregation);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "temp_y2");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("ff", "b2");

    pdb::Handle<pdb::Computation> reluBias =
        pdb::makeObject<FFReluBiasSum>(dropout_rate);
    reluBias->setInput(0, readA);
    reluBias->setInput(1, readB);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>("ff", "y2");
    myWriter->setInput(reluBias);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "ff", "w2");
    print_stats(pdbClient, "ff", "y1");
    print_stats(pdbClient, "ff", "b2");
    print_stats(pdbClient, "ff", "temp_y2");
    print_stats(pdbClient, "ff", "y2");
  }

  pdbClient.deleteSet("ff", "temp_y2");

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "wo");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("ff", "y2");

    pdb::Handle<pdb::Computation> join = pdb::makeObject<FFInputLayerJoin>();
    join->setInput(0, readA);
    join->setInput(1, readB);

    // make the aggregation
    pdb::Handle<pdb::Computation> myAggregation =
        pdb::makeObject<FFAggMatrix>();
    myAggregation->setInput(join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>("ff", "temp_yo");
    myWriter->setInput(myAggregation);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "temp_yo");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("ff", "bo");

    pdb::Handle<pdb::Computation> reluBias =
        pdb::makeObject<FFTransposeBiasSum>();
    reluBias->setInput(0, readA);
    reluBias->setInput(1, readB);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter =
        pdb::makeObject<FFMatrixWriter>("ff", "yo");
    myWriter->setInput(reluBias);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "ff", "wo");
    print_stats(pdbClient, "ff", "y2");
    print_stats(pdbClient, "ff", "bo");
    print_stats(pdbClient, "ff", "temp_yo");
    print_stats(pdbClient, "ff", "yo");

    // print(pdbClient, "ff", "yo");
  }

  pdbClient.deleteSet("ff", "temp_yo");

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "yo");

    pdb::Handle<pdb::Computation> expSum =
        pdb::makeObject<FFRowAggregate>();
    expSum->setInput(readA);

    // make the writer
    pdb::Handle<pdb::Computation> sumWriter =
        pdb::makeObject<FFMatrixWriter>("ff", "yo_exp_sum");
    sumWriter->setInput(expSum);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, sumWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "ff", "yo");
    print_stats(pdbClient, "ff", "yo_exp_sum");

    // print(pdbClient, "ff", "yo");
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        makeObject<FFMatrixBlockScanner>("ff", "yo");
    pdb::Handle<pdb::Computation> readB =
        makeObject<FFMatrixBlockScanner>("ff", "yo_exp_sum");

    pdb::Handle<pdb::Computation> softmax =
        pdb::makeObject<FFOutputLayer>();
    softmax->setInput(0, readA);
    softmax->setInput(1, readB);

    // make the writer
    pdb::Handle<pdb::Computation> sumWriter =
        pdb::makeObject<FFMatrixWriter>("ff", "output");
    sumWriter->setInput(softmax);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, sumWriter)) {
      cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "ff", "yo");
    print_stats(pdbClient, "ff", "yo_exp_sum");
    print_stats(pdbClient, "ff", "output");

    // print(pdbClient, "ff", "yo");
  }

  vector<vector<double>> labels_test;
  load_matrix_from_file(labels_path, labels_test);

  int correct = 0;
  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    auto it = pdbClient.getSetIterator<FFMatrixBlock>("ff", "yo");

    for (auto r : it) {
      double *data = r->getRawDataHandle()->c_ptr();
      int i = 0;
      int j = r->getBlockRowIndex() * r->getRowNums();
      while (i < r->getRowNums() * r->getColNums() && j < labels_test.size()) {
        // double a = exp(data[i]);
        // double b = exp(data[i + 1]);
        // double sum = a + b;

        int pos1 = data[i] > data[i + 1] ? 0 : 1;
        int pos2 = labels_test[j][0] > labels_test[j][1] ? 0 : 1;

        if (pos1 == pos2)
          correct++;

        // cout << (a/sum) << ", " << (b/sum) << " : " << labels_test[j][0] <<
        // ", " << labels_test[j][1] << endl;
        i += r->getColNums();
        j++;
      }
    }

    cout << "Accuracy: " << correct << "/" << labels_test.size() << std::endl;
  }

  sleep(20);

  return 0;
}
