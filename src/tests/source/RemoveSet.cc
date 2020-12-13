//
// by Jia, Apr 2019
//

#include "PDBClient.h"

#include <string>
#include <random>
#include <chrono>

using namespace pdb;




int main(int argc, char* argv[]) {
 
  // we put the error here
  string errMsg;

  // make sure we have the arguments
  if(argc < 4) {
    std::cout << "Usage : ./removeSet managerIP managerPort databaseName setName\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "databaseName - The name of the database\n";
    std::cout << "setName - The name of the set\n";
  }

  //  get the manager address
  std::string managerIp = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  std::string databaseName = std::string(argv[3]);
  std::string setName = std::string(argv[4]);

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, managerIp, clientLogger);
  PDBClient pdbClient(port, managerIp, clientLogger, false, true);

  pdbClient.removeSet(databaseName, setName, errMsg);
  std::cout << "Removed " << databaseName << ":" << setName << std::endl;

}
