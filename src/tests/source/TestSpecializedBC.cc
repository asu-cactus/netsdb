#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "PDBClient.h"
#include "FFMatrixBlockScanner.h"
#include "FFTransposeMult.h"
#include "FFAggMatrix.h"
#include "FFMatrixWriter.h"
#include "FFMatrixBlock.h"
#include "FFMatrixUtil.h"
#include "SpecializedBC.h"
#include "SimpleFF.h"

using namespace std;

int main(int argc, char *argv[]) {

  int rowNum = 569;
  int colNum = 31;
  int block_x = 569;
  int block_y = 31;

  string errMsg;

  makeObjectAllocatorBlock(1024 * 1024 * 1024, true);

  string masterIp = "localhost";
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("DecisionTreeClientLog");
  pdb::PDBClient pdbClient(8108, masterIp, clientLogger, false, true);
  pdb::CatalogClient catalogClient(8108, masterIp, clientLogger);

  ff::createDatabase(pdbClient, "decisiontreeBC");

  ff::loadLibrary(pdbClient, "libraries/libFFMatrixMeta.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixData.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlock.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixBlockScanner.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixWriter.so");
  ff::loadLibrary(pdbClient, "libraries/libFFMatrixPartitioner.so");

  ff::createSet(pdbClient, "decisiontreeBC", "inputs", "inputs", 64);
  ff::createSet(pdbClient, "decisiontreeBC", "labels", "labels", 64);

  std::cout << "To load shared libraries of rules" << std::endl;
  ff::loadLibrary(pdbClient, "libraries/libSpecializedBC.so");
  
  auto begin = std::chrono::high_resolution_clock::now();

  std::cout << "To load matrix for decision tree inputs" << std::endl;
  ff::loadMatrix(pdbClient, "decisiontreeBC", "inputs", rowNum, colNum, block_x,
                   block_y, false, false, errMsg);

  std::cout << "To print the inputs" << std::endl;
  ff::print(pdbClient, "decisiontreeBC", "inputs");

  pdb::Handle<pdb::Computation> inputMatrix = pdb::makeObject<FFMatrixBlockScanner>("decisiontreeBC", "inputs");

  std::cout << "To make object of decision tree shared libraries" << std::endl;
  pdb::Handle<pdb::Computation> specializedBC = pdb::makeObject<SpecializedBC>();
  specializedBC->setInput(inputMatrix);

  std::cout << "To set the Computation" << std::endl;
  pdb::Handle<pdb::Computation> labelWriter = nullptr;
  labelWriter = pdb::makeObject<FFMatrixWriter>("decisiontreeBC", "labels");
  labelWriter->setInput(specializedBC);

  bool materializeHash = false;
  std::cout << "To run the Computation" << std::endl;
  auto exe_begin = std::chrono::high_resolution_clock::now();

  if (!pdbClient.executeComputations(errMsg, "decisiontreeBC", materializeHash, labelWriter)) {
    cout << "Computation failed. Message was: " << errMsg << "\n";
    exit(1);
  }

  auto end = std::chrono::high_resolution_clock::now();

  std::cout << "****Decision Tree End-to-End Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  std::cout << "****Decision Tree Execution Time Duration: ****"
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - exe_begin).count()
              << " secs." << std::endl;

  //verify the results
  std::cout << "To print the status" << std::endl;
  ff::print_stats(pdbClient, "decisiontreeBC", "labels");
  std::cout << "To print the results" << std::endl;
  ff::print(pdbClient, "decisiontreeBC", "labels");
  return 0;
}