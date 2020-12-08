#include "PDBClient.h"
#include "RedditComment.h"

#include <string>

int main(int argc, char* argv[]) {
 
  // we put the error here
  std::string errMsg;
  // make sure we have the arguments
  if(argc < 2) {
    std::cout << "Usage : ./LoadRedditComments managerIP managerPort\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";  
  }
  //  get the manager address
  std::string managerIp = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, managerIp, clientLogger);
  PDBClient pdbClient(port, managerIp, clientLogger, false, true);
  pdbClient.registerType("libraries/libRedditComment.so", errMsg);
  // now, create a new database
  pdbClient.createDatabase("redditDB", errMsg);
  Handle<LambdaIdentifier> myLambda1 = nullptr;
  // now, create the output set
  pdbClient.removeSet("redditDB", "comments", errMsg);
  pdbClient.createSet<reddit::Comment>("redditDB", "comments", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "comments", nullptr, myLambda1);
}
