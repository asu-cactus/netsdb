//
// by Jia, Apr 2019
//

#include "PDBClient.h"
#include "RedditComment.h"

#include <string>
#include <random>
#include <chrono>

using namespace pdb;

void parseInputJSONFile(PDBClient &pdbClient, std::string fileName, int blockSizeInMB) {

  // the error message is put there
  string errMsg;
  std ::ifstream inFile(fileName.c_str());
  std :: string line;
  bool end = false;
  bool rollback = false;

  pdb::makeObjectAllocatorBlock((size_t)blockSizeInMB * (size_t)1024 * (size_t)1024, true);
  pdb::Handle<pdb::Vector<pdb::Handle<reddit::Comment>>> storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<reddit::Comment>>> ();
  while (!end) {
      if (!rollback) {
          if(!std::getline(inFile, line)){
             end = true;
             if (! pdbClient.sendData<reddit::Comment> (std::pair<std::string, std::string>("comments", "redditDB"), storeMe, errMsg)) {
                 std::cout << "Failed to send data to dispatcher server" << std::endl;
                 return;
             }
             pdbClient.flushData (errMsg);
             std::cout << "Dispatched " << storeMe->size() << " comments." << std::endl;
             break;
          }
      }
      rollback = false; 
      try {
          pdb::Handle<reddit::Comment> comment = pdb::makeObject<reddit::Comment>(line);
          storeMe->push_back(comment);
      }
      catch (pdb::NotEnoughSpace &n) {
          if (! pdbClient.sendData<reddit::Comment> (std::pair<std::string, std::string>("comments", "redditDB"), storeMe, errMsg)) {
             std::cout << "Failed to send data to dispatcher server" << std::endl;
             return;
          }
          pdbClient.flushData (errMsg);
          std::cout << "Dispatched " << storeMe->size() << " comments." << std::endl;
          rollback = true; 
          pdb::makeObjectAllocatorBlock((size_t)blockSizeInMB * (size_t)1024 * (size_t)1024, true);
          storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<reddit::Comment>>> ();

      }
   }

}



int main(int argc, char* argv[]) {
 
  // we put the error here
  string errMsg;

  // make sure we have the arguments
  if(argc < 5) {

    std::cout << "Usage : ./LoadRedditComments managerIP managerPort inputFileName whetherToPartitionData, whetherToRegisterLibraries\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "inputFileName - The file to load for reddit comments data, which is a set of JSON objects\n";
    std::cout << "whetherToPrepartitionData - Y yes, N no\n";
    std::cout << "whetherToRegisterLibraries - Y yes, N no\n";    
  }

  //  get the manager address
  std::string managerIp = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  std::string inputFileName = std::string(argv[3]);
  bool whetherToPartitionData = true;
  if (strcmp(argv[4], "N")==0) {
      whetherToPartitionData = false;
  }
  bool whetherToRegisterLibraries = true;
  if (strcmp(argv[5], "N")==0) {
      whetherToRegisterLibraries = false;
  }

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, managerIp, clientLogger);
  PDBClient pdbClient(port, managerIp, clientLogger, false, true);
  pdbClient.registerType("libraries/libRedditComment.so", errMsg);

  // now, create a new database
  pdbClient.createDatabase("redditDB", errMsg);
  
  Handle<LambdaIdentifier> myLambda1 = nullptr;
  /*
  if (whetherToPartitionData) {
      myLambda1 = makeObject<LambdaIdentifier>("pageRankIteration_1", "JoinComp_2", "attAccess_1");
  }
  */

  // now, create the output set
  pdbClient.removeSet("redditDB", "comments", errMsg);
  pdbClient.createSet<reddit::Comment>("redditDB", "comments", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "comments", nullptr, myLambda1);

  // parse the input file 
  parseInputJSONFile(pdbClient, inputFileName, 64); }
