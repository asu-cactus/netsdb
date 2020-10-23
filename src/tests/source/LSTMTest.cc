#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "FFMatrixBlock.h"
#include "FFMatrixMeta.h"
#include "FFMatrixData.h"

#include "FFMatrixBlockScanner.h"
#include "FFMatrixWriter.h"

#include "FFInputLayerJoin.h"
#include "FFAggMatrix.h"

#include "LSTMThreeWaySum.h"

#include "PDBClient.h"

using namespace std;

void print(pdb::PDBClient &pdbClient, string dbName, string setName) {
  auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);

  for (auto r : it) {
    double *data = r->getRawDataHandle()->c_ptr();
    for (int i = 0; i < r->getRowNums() * r->getColNums(); i++) {
      std::cout << data[i] << ",";
    }
  }
}

void print_stats(pdb::PDBClient &pdbClient, string dbName, string setName) {
  int rows = 0, cols = 0, blocks = 0;
  int totalRows = 0, totalCols = 0;
  int blockRows = 0, blockCols = 0;
  auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);

  for (auto r : it) {
    std::cout << r->getBlockRowIndex() << "," << r->getBlockColIndex() << ";";
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

  std::cout << "\n"
            << setName << " (" << blockRows << " X " << blockCols << ") ("
            << blocks << ") : (" << totalRows << " x " << totalCols
            << "), Each block size: " << rows << " x " << cols << std::endl;
}

void loadMatrix(pdb::PDBClient &pdbClient, String dbName, String setName,
                int totalX, int totalY, int blockX, int blockY, int initVal,
                std::string &errMsg) {

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
                    : initVal == -1 ? i + j + ii + jj : initVal;
            (*(myData->getRawDataHandle()))[ii * blockY + jj] = data;
          }
        }

        // std::cout << "New block: " << total << std::endl;
        storeMatrix1->push_back(myData);
        total++;
      }
    }
    if (!pdbClient.sendData<FFMatrixBlock>(
            std::pair<std::string, std::string>(setName, dbName), storeMatrix1,
            errMsg)) {
      std::cout << "Failed to send data to dispatcher server" << std::endl;
      exit(1);
    }
  } catch (NotEnoughSpace &e) {
    std::cout << "Failed to send data to dispatcher server" << std::endl;
    exit(1);
  }
  std::cout << setName << "(" << totalX << "x" << totalY << "): (" << numXBlocks
            << " x " << numYBlocks << ")" << total << " blocks = " << blockX
            << " x " << blockY << " each" << std::endl;

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

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("LSTMclientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
  loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
  loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");

  loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
  loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");

  loadLibrary(pdbClient, "libraries/libFFInputLayerJoin.so");
  loadLibrary(pdbClient, "libraries/libFFAggMatrix.so");

  loadLibrary(pdbClient, "libraries/libLSTMThreeWaySum.so");


  createDatabase(pdbClient, "lstm");

  createSet(pdbClient, "lstm", "forget", "Forget");
  createSet(pdbClient, "lstm", "input", "Input");
  createSet(pdbClient, "lstm", "output", "Output");
  createSet(pdbClient, "lstm", "cell_state", "CellState");
  createSet(pdbClient, "lstm", "cell_state_temp", "CellStateTemp");
  createSet(pdbClient, "lstm", "hidden_state", "HiddenState");

  createSet(pdbClient, "lstm", "w_forget", "WForget");
  createSet(pdbClient, "lstm", "w_forget_x", "WForgetX");
  createSet(pdbClient, "lstm", "w_input", "WInput");
  createSet(pdbClient, "lstm", "w_input_x", "WInputX");
  createSet(pdbClient, "lstm", "w_output", "WOutput");
  createSet(pdbClient, "lstm", "w_output_x", "WOutputX");
  createSet(pdbClient, "lstm", "w_cell_state", "WCellState");

  createSet(pdbClient, "lstm", "u_forget", "UForget");
  createSet(pdbClient, "lstm", "u_forget_h", "UForgetH");
  createSet(pdbClient, "lstm", "u_input", "UInput");
  createSet(pdbClient, "lstm", "u_input_h", "UInputH");
  createSet(pdbClient, "lstm", "u_output", "UOutput");
  createSet(pdbClient, "lstm", "u_output_h", "UOutputH");
  createSet(pdbClient, "lstm", "u_cell_state", "UCellState");
  createSet(pdbClient, "lstm", "u_cell_state_h", "UCellStateH");

  createSet(pdbClient, "lstm", "b_forget", "BForget");
  createSet(pdbClient, "lstm", "b_input", "BInput");
  createSet(pdbClient, "lstm", "b_output", "BOutput");
  createSet(pdbClient, "lstm", "b_cell_state", "BCellState");

  createSet(pdbClient, "lstm", "cell_state_next", "CellStatePrev");
  createSet(pdbClient, "lstm", "hidden_state_next", "HiddenStateNext");

  int D = 400; // features
  int B = 500; // batch size
  int L = 128; // output labels?
  int block_x = 100;
  int block_y = 100;

  loadMatrix(pdbClient, "lstm", "w_forget", L, D, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "input", D, B, block_x, block_y, -1, errMsg);
  loadMatrix(pdbClient, "lstm", "u_forget", L, L, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "b_forget", L, B, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "hidden_state", L, B, block_x, block_y, 0, errMsg);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "w_forget");
    pdb::Handle<pdb::Computation> readB =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "input");

    // multiply
    Handle<Computation> myMultiply1Join = makeObject<FFInputLayerJoin>();
    myMultiply1Join->setInput(0, readA);
    myMultiply1Join->setInput(1, readB);

    Handle<Computation> myMultiply2Aggregate = makeObject<FFAggMatrix>();
    myMultiply2Aggregate->setInput(myMultiply1Join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("lstm", "w_forget_x");
    myWriter->setInput(myMultiply2Aggregate);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "lstm", "w_forget");
    print_stats(pdbClient, "lstm", "input");
    print_stats(pdbClient, "lstm", "w_forget_x");
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "u_forget");
    pdb::Handle<pdb::Computation> readB =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "hidden_state");

    // multiply
    Handle<Computation> myMultiply1Join = makeObject<FFInputLayerJoin>();
    myMultiply1Join->setInput(0, readA);
    myMultiply1Join->setInput(1, readB);

    Handle<Computation> myMultiply2Aggregate = makeObject<FFAggMatrix>();
    myMultiply2Aggregate->setInput(myMultiply1Join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("lstm", "u_forget_h");
    myWriter->setInput(myMultiply2Aggregate);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "lstm", "u_forget");
    print_stats(pdbClient, "lstm", "hidden_state");
    print_stats(pdbClient, "lstm", "u_forget_h");
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    // make the computation
    pdb::Handle<pdb::Computation> readA =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "w_forget_x");
    pdb::Handle<pdb::Computation> readB =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "u_forget_h");
    pdb::Handle<pdb::Computation> readC =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "b_forget");

    // add and sigmod
    pdb::Handle<pdb::Computation> sum = pdb::makeObject<LSTMThreeWaySum>();
    sum->setInput(0, readA);
    sum->setInput(1, readB);
    sum->setInput(2, readC);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("lstm", "forget");
    myWriter->setInput(sum);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "lstm", "w_forget_x");
    print_stats(pdbClient, "lstm", "u_forget_h");
    print_stats(pdbClient, "lstm", "b_forget");
    print_stats(pdbClient, "lstm", "forget");
  }

  return 0;
}
