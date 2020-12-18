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
#include "LSTMTwoSum.h"
#include "LSTMHiddenState.h"

#include "PDBClient.h"

using namespace std;

void print(pdb::PDBClient &pdbClient, string dbName, string setName) {
  auto it = pdbClient.getSetIterator<FFMatrixBlock>(dbName, setName);

  for (auto r : it) {
    std::cout << "hash:"<< r->hash() <<std::endl;
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
  loadLibrary(pdbClient, "libraries/libLSTMTwoSum.so");
  loadLibrary(pdbClient, "libraries/libLSTMHiddenState.so");


  createDatabase(pdbClient, "lstm");


  createSet(pdbClient, "lstm", "b_f", "BForget");
  createSet(pdbClient, "lstm", "b_1", "BInput");
  createSet(pdbClient, "lstm", "b_2", "BOutput");

  int D = 400; // features
  int B = 500; // batch size
  int L = 128; // output labels?
  int block_x = 100;
  int block_y = 100;

  loadMatrix(pdbClient, "lstm", "b_f", L, B, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "b_1", L, B, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "b_2", L, B, block_x, block_y, 0, errMsg);

  print(pdbClient, "lstm", "b_f");
  print(pdbClient, "lstm", "b_1");
  print(pdbClient, "lstm", "b_2");


  {
    pdb::Handle<pdb::Computation> w_f =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "b_1");
    pdb::Handle<pdb::Computation> x_t =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "b_2");
    pdb::Handle<pdb::Computation> b_f =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "b_f");

    // add and sigmod
    pdb::Handle<pdb::Computation> sum = pdb::makeObject<LSTMThreeWaySum>();
    sum->setInput(0, w_f);
    sum->setInput(1, x_t);
    sum->setInput(2, b_f);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("lstm", "f_t");
    myWriter->setInput(sum);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }
  return 0;

/*  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "lstm", "w_f");
    print_stats(pdbClient, "lstm", "x_t");
    print_stats(pdbClient, "lstm", "u_f");
    print_stats(pdbClient, "lstm", "h_t_1");
    print_stats(pdbClient, "lstm", "b_f");
    print_stats(pdbClient, "lstm", "f_t");
  }

  loadMatrix(pdbClient, "lstm", "w_i", L, D, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "u_i", L, L, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "b_i", L, B, block_x, block_y, 0, errMsg);

  {
    pdb::Handle<pdb::Computation> w_i =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "w_i");
    pdb::Handle<pdb::Computation> x_t =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "x_t");

    // multiply
    Handle<Computation> w_input_x_join = makeObject<FFInputLayerJoin>();
    w_input_x_join->setInput(0, w_i);
    w_input_x_join->setInput(1, x_t);

    Handle<Computation> w_input_x_agg = makeObject<FFAggMatrix>();
    w_input_x_agg->setInput(w_input_x_join);


    pdb::Handle<pdb::Computation> u_i =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "u_i");
    pdb::Handle<pdb::Computation> h_t_1 =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "h_t_1");

    Handle<Computation> u_input_h_join = makeObject<FFInputLayerJoin>();
    u_input_h_join->setInput(0, u_i);
    u_input_h_join->setInput(1, h_t_1);

    Handle<Computation> u_input_h_agg = makeObject<FFAggMatrix>();
    u_input_h_agg->setInput(u_input_h_join);


    pdb::Handle<pdb::Computation> b_i =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "b_i");

    // add and sigmod
    pdb::Handle<pdb::Computation> sum = pdb::makeObject<LSTMThreeWaySum>();
    sum->setInput(0, w_input_x_agg);
    sum->setInput(1, u_input_h_agg);
    sum->setInput(2, b_i);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("lstm", "i_t");
    myWriter->setInput(sum);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "lstm", "w_i");
    print_stats(pdbClient, "lstm", "x_t");
    print_stats(pdbClient, "lstm", "u_i");
    print_stats(pdbClient, "lstm", "h_t_1");
    print_stats(pdbClient, "lstm", "b_i");
    print_stats(pdbClient, "lstm", "i_t");
  }

  loadMatrix(pdbClient, "lstm", "w_o", L, D, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "u_o", L, L, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "b_o", L, B, block_x, block_y, 0, errMsg);

  {
    pdb::Handle<pdb::Computation> w_o =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "w_o");
    pdb::Handle<pdb::Computation> x_t =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "x_t");

    // multiply
    Handle<Computation> w_output_x_join = makeObject<FFInputLayerJoin>();
    w_output_x_join->setInput(0, w_o);
    w_output_x_join->setInput(1, x_t);

    Handle<Computation> w_output_x_agg = makeObject<FFAggMatrix>();
    w_output_x_agg->setInput(w_output_x_join);


    pdb::Handle<pdb::Computation> u_o =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "u_o");
    pdb::Handle<pdb::Computation> h_t_1 =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "h_t_1");

    Handle<Computation> u_output_h_join = makeObject<FFInputLayerJoin>();
    u_output_h_join->setInput(0, u_o);
    u_output_h_join->setInput(1, h_t_1);

    Handle<Computation> u_output_h_agg = makeObject<FFAggMatrix>();
    u_output_h_agg->setInput(u_output_h_join);


    pdb::Handle<pdb::Computation> b_o =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "b_o");

    // add and sigmod
    pdb::Handle<pdb::Computation> sum = pdb::makeObject<LSTMThreeWaySum>();
    sum->setInput(0, w_output_x_agg);
    sum->setInput(1, u_output_h_agg);
    sum->setInput(2, b_o);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("lstm", "o_t");
    myWriter->setInput(sum);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "lstm", "w_o");
    print_stats(pdbClient, "lstm", "x_t");
    print_stats(pdbClient, "lstm", "u_o");
    print_stats(pdbClient, "lstm", "h_t_1");
    print_stats(pdbClient, "lstm", "b_o");
    print_stats(pdbClient, "lstm", "o_t");
  }

  loadMatrix(pdbClient, "lstm", "w_c", L, D, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "u_c", L, L, block_x, block_y, 0, errMsg);
  loadMatrix(pdbClient, "lstm", "b_c", L, B, block_x, block_y, 0, errMsg);

  {
    pdb::Handle<pdb::Computation> w_c =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "w_c");
    pdb::Handle<pdb::Computation> x_t =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "x_t");

    // multiply
    Handle<Computation> w_cell_state_join = makeObject<FFInputLayerJoin>();
    w_cell_state_join->setInput(0, w_c);
    w_cell_state_join->setInput(1, x_t);

    Handle<Computation> w_cell_state_agg = makeObject<FFAggMatrix>();
    w_cell_state_agg->setInput(w_cell_state_join);


    pdb::Handle<pdb::Computation> u_c =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "u_c");
    pdb::Handle<pdb::Computation> h_t_1 =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "h_t_1");

    Handle<Computation> u_cell_state_h_join = makeObject<FFInputLayerJoin>();
    u_cell_state_h_join->setInput(0, u_c);
    u_cell_state_h_join->setInput(1, h_t_1);

    Handle<Computation> u_cell_state_h_agg = makeObject<FFAggMatrix>();
    u_cell_state_h_agg->setInput(u_cell_state_h_join);


    pdb::Handle<pdb::Computation> b_c =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "b_c");

    // add and tanh
    pdb::Handle<pdb::Computation> sum = pdb::makeObject<LSTMThreeWaySum>(SumActivation::Tanh);
    sum->setInput(0, w_cell_state_agg);
    sum->setInput(1, u_cell_state_h_agg);
    sum->setInput(2, b_c);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("lstm", "c_t_temp");
    myWriter->setInput(sum);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "lstm", "w_c");
    print_stats(pdbClient, "lstm", "x_t");
    print_stats(pdbClient, "lstm", "u_c");
    print_stats(pdbClient, "lstm", "h_t_1");
    print_stats(pdbClient, "lstm", "b_c");
    print_stats(pdbClient, "lstm", "c_t_temp");
  }

  loadMatrix(pdbClient, "lstm", "c_t_1", L, B, block_x, block_y, 0, errMsg);

  {
    pdb::Handle<pdb::Computation> f_t =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "f_t");
    pdb::Handle<pdb::Computation> c_t_1 =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "c_t_1");

    // multiply
    Handle<Computation> forget_cell_state_join = makeObject<FFInputLayerJoin>();
    forget_cell_state_join->setInput(0, f_t);
    forget_cell_state_join->setInput(1, c_t_1);

    Handle<Computation> forget_cell_state_agg = makeObject<FFAggMatrix>();
    forget_cell_state_agg->setInput(forget_cell_state_join);

    pdb::Handle<pdb::Computation> i_t =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "i_t");
    pdb::Handle<pdb::Computation> c_t_temp =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "c_t_temp");

    Handle<Computation> input_cell_state_temp_join = makeObject<FFInputLayerJoin>();
    input_cell_state_temp_join->setInput(0, i_t);
    input_cell_state_temp_join->setInput(1, c_t_temp);

    Handle<Computation> input_cell_state_temp_agg = makeObject<FFAggMatrix>();
    input_cell_state_temp_agg->setInput(input_cell_state_temp_join);

    // add
    pdb::Handle<pdb::Computation> sum = pdb::makeObject<LSTMTwoSum>();
    sum->setInput(0, forget_cell_state_agg);
    sum->setInput(1, input_cell_state_temp_agg);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("lstm", "c_t");
    myWriter->setInput(sum);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "lstm", "f_t");
    print_stats(pdbClient, "lstm", "c_t_1");
    print_stats(pdbClient, "lstm", "i_t");
    print_stats(pdbClient, "lstm", "c_t_temp");
    print_stats(pdbClient, "lstm", "c_t");
  }

  {
    pdb::Handle<pdb::Computation> o_t =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "o_t");
    pdb::Handle<pdb::Computation> c_t_1 =
        pdb::makeObject<FFMatrixBlockScanner>("lstm", "c_t");

    // multiply
    Handle<Computation> hidden_state_join = makeObject<LSTMHiddenState>();
    hidden_state_join->setInput(0, o_t);
    hidden_state_join->setInput(1, c_t_1);

    Handle<Computation> hidden_state_agg = makeObject<FFAggMatrix>();
    hidden_state_agg->setInput(hidden_state_join);

    // make the writer
    pdb::Handle<pdb::Computation> myWriter = pdb::makeObject<FFMatrixWriter>("lstm", "h_t");
    myWriter->setInput(hidden_state_agg);

    // run the computation
    if (!pdbClient.executeComputations(errMsg, myWriter)) {
      std::cout << "Computation failed. Message was: " << errMsg << "\n";
      return 1;
    }
  }

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

    print_stats(pdbClient, "lstm", "o_t");
    print_stats(pdbClient, "lstm", "c_t");
    print_stats(pdbClient, "lstm", "h_t");
  }
*/

  return 0;
}
