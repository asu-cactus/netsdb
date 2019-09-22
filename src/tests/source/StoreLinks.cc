//
// Created by dimitrije on 9/18/19.
//

#include "PDBClient.h"
#include "Link.h"

#include <string>
#include <random>
#include <chrono>

using namespace pdb;

void storeLinks(PDBClient &pdbClient, size_t numNodes, size_t maxLinks) {

  // the error message is put there
  string errMsg;

  // init the random stuff
  std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
  std::uniform_int_distribution<size_t> distribution(0, maxLinks);
  std::uniform_int_distribution<size_t> linkDistribution(0, numNodes);

  pdb::Handle<pdb::Vector<pdb::Handle<Link>>> storeMe;
  for(size_t i = 0; i < numNodes;) {

    // make a block of a size
    pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

    // the vector we want to store
    storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<Link>>>();

    // at the beginning stuff is not sent
    bool isSent = false;

    try {

      for(; i < numNodes; i++) {

        // make my identifier
        std::string me = std::to_string(i);

        // generate the links
        size_t numLinks = distribution(generator);
        for(size_t j = 0; j < numLinks; ++j) {
          storeMe->push_back(pdb::makeObject<Link>(me, std::to_string(linkDistribution(generator))));
        }
      }

    } catch (pdb::NotEnoughSpace &n) {

      // send the data
      pdbClient.sendData<Link>(std::pair<std::string, std::string>("links", "db"), storeMe, errMsg);

      // mark this as setn
      isSent = true;
    }

    if(!isSent) {

      // send the data
      pdbClient.sendData<Link>(std::pair<std::string, std::string>("links", "db"), storeMe, errMsg);
    }
  }

  // to write back all buffered records
  pdbClient.flushData(errMsg);
}

int main(int argc, char* argv[]) {
 
  // we put the error here
  string errMsg;

  // make sure we have the arguments
  if(argc != 5) {

    std::cout << "Usage : ./TestSparseMultiply managerIP managerPort numNodes maxLinks\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "numNodes - The number of nodes in the system\n";
    std::cout << "maxLinks - The maximum number of links per node\n";
  }

  //  get the manager address
  std::string managerIp = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  int32_t numNodes = std::stoi(argv[3]);
  int32_t maxLinks = std::stoi(argv[4]);

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, managerIp, clientLogger);
  PDBClient pdbClient(port, managerIp, clientLogger, false, true);

  pdbClient.registerType("libraries/libLink.so", errMsg);

  // now, create a new database
  pdbClient.createDatabase("db", errMsg);

  // now, create the output set
  pdbClient.removeSet("db", "links", errMsg);
  pdbClient.createSet<Link>("db", "links", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadLinks");

  // store all the links
  storeLinks(pdbClient, numNodes, maxLinks);
}
