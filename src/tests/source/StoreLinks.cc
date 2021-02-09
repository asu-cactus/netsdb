//
// Created by dimitrije on 9/18/19.
// Significantly modified by Jia to change FROM->TO (src->dest) representation to URL->NEIGHBORS (src-> dests) representation on 10/12/19.
//
//

#include "PDBClient.h"
#include "Link.h"
#include "RankedUrl.h"

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

        // generate the link
	Handle<Link> me = makeObject<Link>(i);
        size_t numNeighbors = distribution(generator);
        for(size_t j = 0; j < numNeighbors; ++j) {
	    me->addNeighbor(linkDistribution(generator));
        }
        storeMe->push_back(me);
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



void storeRanks(PDBClient &pdbClient, size_t numNodes) {

  // the error message is put there
  string errMsg;


  pdb::Handle<pdb::Vector<pdb::Handle<RankedUrl>>> storeMe;
  for(size_t i = 0; i < numNodes;) {

    // make a block of a size
    pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

    // the vector we want to store
    storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<RankedUrl>>>();

    // at the beginning stuff is not sent
    bool isSent = false;

    try {

      for(; i < numNodes; i++) {

        // generate the link
        Handle<RankedUrl> me = makeObject<RankedUrl>(i, 1);
        storeMe->push_back(me);
      }

    } catch (pdb::NotEnoughSpace &n) {

      // send the data
      pdbClient.sendData<RankedUrl>(std::pair<std::string, std::string>("rankings_0", "db"), storeMe, errMsg);

      // mark this as setn
      isSent = true;
    }

    if(!isSent) {

      // send the data
      pdbClient.sendData<RankedUrl>(std::pair<std::string, std::string>("rankings_0", "db"), storeMe, errMsg);
    }
  }

  // to write back all buffered records
  pdbClient.flushData(errMsg);
}




int main(int argc, char* argv[]) {
 
  // we put the error here
  string errMsg;

  // make sure we have the arguments
  if(argc < 5) {

    std::cout << "Usage : ./TestSparseMultiply managerIP managerPort numNodes maxLinks whetherToPartitionData whetherToRegisterLibraries\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "numNodes - The number of nodes in the system\n";
    std::cout << "maxLinks - The maximum number of links per node\n";
    std::cout << "whetherToPrepartitionData - Y yes, N no\n";
    std::cout << "whetherToRegisterLibraries - Y yes, N no\n";

  }

  //  get the manager address
  std::string managerIp = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  int32_t numNodes = std::stoi(argv[3]);
  int32_t maxLinks = std::stoi(argv[4]);
  bool whetherToPartitionData = true;
  if (strcmp(argv[5], "N")==0) {
      whetherToPartitionData = false;
  }
  bool whetherToRegisterLibraries = true;
  if (strcmp(argv[6], "N")==0) {
      whetherToRegisterLibraries = false;
  }

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, managerIp, clientLogger);
  PDBClient pdbClient(port, managerIp, clientLogger, false, true);
      pdbClient.registerType("libraries/libLink.so", errMsg);
      pdbClient.registerType("libraries/libLinkPartition.so", errMsg);
      pdbClient.registerType("libraries/libRankPartition.so", errMsg);
      pdbClient.registerType("libraries/libURLRankMultiSelection.so", errMsg);
      pdbClient.registerType("libraries/libDistinctProjection.so", errMsg);
      pdbClient.registerType("libraries/libURLURLsRank.so", errMsg);
      pdbClient.registerType("libraries/libJoinRankedUrlWithLink.so", errMsg);
      pdbClient.registerType("libraries/libLinkScanner.so", errMsg);
      pdbClient.registerType("libraries/libLinkWithValue.so", errMsg);
      pdbClient.registerType("libraries/libRankedUrl.so", errMsg);
      pdbClient.registerType("libraries/libRankedUrlScanner.so", errMsg);
      pdbClient.registerType("libraries/libRankedUrlWriter.so", errMsg);
      pdbClient.registerType("libraries/libRankUpdateAggregation.so", errMsg);

  // now, create a new database
  pdbClient.createDatabase("db", errMsg);



  Handle<LambdaIdentifier> myLambda1 = nullptr;
  if (whetherToPartitionData) {
      myLambda1 = makeObject<LambdaIdentifier>("pageRankIteration_1", "JoinComp_2", "attAccess_1");
  }
  // now, create the output set
  pdbClient.removeSet("db", "rankings_0", errMsg);
  pdbClient.createSet<RankedUrl>("db", "rankings_0", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "rankings_0", nullptr, myLambda1);
  pdbClient.removeSet("db", "rankings_1", errMsg);
  pdbClient.createSet<RankedUrl>("db", "rankings_1", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "rankings_1", nullptr, myLambda1);

  // now, create the output set
  //
  pdbClient.removeSet("db", "links", errMsg);
  Handle<LambdaIdentifier> myLambda = nullptr;
  if (whetherToPartitionData) {
      myLambda = makeObject<LambdaIdentifier>("pageRankIteration_1", "JoinComp_2", "attAccess_0");
  }

  pdbClient.createSet<Link>("db", "links", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadLinks", nullptr, myLambda);

  // store all the links
  storeLinks(pdbClient, numNodes, maxLinks);
  storeRanks(pdbClient, numNodes);
}
