#include <iostream>
#include <cstring>

#include <PDBClient.h>
#include <RedditComment.h>
#include <RedditAuthor.h>
#include <RedditFeatures.h>
#include <RedditJoin.h>
#include <ScanUserSet.h>
#include <WriteUserSet.h>

using namespace pdb;



int main(int argc, char* argv[]) {

  // this is where we put an eventual error message
  string errMsg;

  // make sure we have the arguments
  if(argc < 3) {

    std::cout << "Usage : ./TestRedditJoin managerIP managerPort whetherToPartitionData\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
  }

  //  get the manager address
  std::string ip = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, ip, clientLogger);
  PDBClient pdbClient(port, ip, clientLogger, false, true);
  
  

  // print the results
  std::cout << "Authors: \n";
  SetIterator<reddit::Author> result = pdbClient.getSetIterator<reddit::Author>("redditDB", "authors");
  int count = 0;
  for (const auto &r : result) {
     std::cout << r->author << ": " << Hasher<Handle<reddit::Author>>::hash(r) << std::endl;
     count++;
  }
  std::cout << "count: " << count << std::endl;

}
