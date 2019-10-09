#include <iostream>
#include <cstring>
#include "PDBClient.h"
#include "SparseMultiplyAggregate.h"
#include "MatrixSparse.h"
#include "SparseMultiplyJoin.h"
#include "LAScanMatrixBlockSet.h"
#include "LAWriteMatrixBlockSet.h"
#include "Lambda.h"

using namespace pdb;


// type to indicate what type of matrix we are storing
enum class MatrixStorageType {

  ROW_MATRIX,
  COL_MATRIX
};

int main(int argc, char* argv[]) {

  // make sure we have the arguments
  if(argc != 5) {

    std::cout << "Usage : ./TestSparseMultiply managerIP managerPort\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
  }

  // this is where we put an eventual error message
  string errMsg;
 bool run;
 bool removeData;
  //  get the manager address
  std::string ip = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
if(strcmp(argv[3],"N") == 0) {
        run =false;
            std::cout << "You successfully disabled  running" << std::endl;
        } else {
           run  =true;
        }
 if(strcmp(argv[4],"N") == 0) {
        removeData =false;
            std::cout << "You successfully disabled remove data" << std::endl;
        } else {
           removeData  =true;
        } 



  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  PDBClient pdbClient(port, ip, clientLogger, false, true);
  CatalogClient catalogClient(port, ip, clientLogger);

  // now, register a type for user data
  pdbClient.registerType("libraries/libMatrixSparse.so", errMsg);
  pdbClient.registerType("libraries/libSparseMultiplyJoin.so", errMsg);
  pdbClient.registerType("libraries/libSparseMultiplyAggregate.so", errMsg);
  pdbClient.registerType("libraries/libLAScanMatrixBlockSet.so", errMsg);
  pdbClient.registerType("libraries/libLAWriteMatrixBlockSet.so", errMsg);
 if(run==true){
  // now, create a new database
  pdbClient.createDatabase("db", errMsg);

  // now, create the output set
  pdbClient.createSet<MatrixSparse>("db", "c", errMsg);

  // setup the query
  const UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};

  // connect to the query client
  Handle<Computation> setA = makeObject<LAScanMatrixBlockSet>("db", "a");
  Handle<Computation> setB = makeObject<LAScanMatrixBlockSet>("db", "b");
//  auto begin = std::chrono::high_resolution_clock::now();

  // make the multiply join
  Handle<Computation> myQuery = makeObject<SparseMultiplyJoin>();
  myQuery->setInput(0, setA);
  myQuery->setInput(1, setB);
 
 Handle<Computation> myMultiply2Aggregate = makeObject<SparseMultiplyAggregate>();
  myMultiply2Aggregate->setInput(myQuery);
  // writes out the block
  Handle<Computation> myWriteSet = makeObject<LAWriteMatrixBlockSet>("db", "c");
  myWriteSet->setInput(myMultiply2Aggregate);
  auto begin = std::chrono::high_resolution_clock::now();
 //execute the computation
  pdbClient.executeComputations(errMsg, myWriteSet);
  auto end = std::chrono::high_resolution_clock::now();
std::cout << "Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
 
}
if(removeData == true){

        if (!pdbClient.removeSet("db", "a", errMsg)) {
            cout << "Not able to remove set 1: " + errMsg;
            exit(-1);
        } else {
            cout << "Removed set 1.\n";
        }

       if (!pdbClient.removeSet("db", "b", errMsg)) {
            cout << "Not able to remove set 2: " + errMsg;
            exit(-1);
        } else {
            cout << "Removed set 2.\n";
        }
 }
    int code = system("scripts/cleanupSoFiles.sh");
    if (code < 0) {
        std::cout << "Can't cleanup so files" << std::endl;
    }


 // print the results


}
