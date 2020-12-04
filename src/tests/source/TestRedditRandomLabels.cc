#include <iostream>
#include <cstring>

#include <PDBClient.h>
#include <RedditComment.h>
#include <RedditLabelProjection.h>
#include <ScanUserSet.h>
#include <WriteUserSet.h>

using namespace pdb;


void run(PDBClient &pdbClient) {

  string errMsg;

    pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

    // clear the rankings
    pdbClient.clearSet("redditDB", "labeledComments", "reddit::Comment", errMsg);

    // make a scan set
    Handle<Computation> input = makeObject<ScanUserSet<reddit::Comment>>("redditDB", "comments");

    // join previous ranks with links
    Handle<Computation> select = makeObject<reddit::RedditLabelProjection>(0.5);
    select->setInput(input);


    // write it out
    Handle<Computation> myWriteSet = makeObject<WriteUserSet<reddit::Comment>>("redditDB", "labeledComments");
    myWriteSet->setInput(select);

    // execute the computation
    auto begin = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "reddit", myWriteSet);
    auto end = std::chrono::high_resolution_clock::now();
          std::cout << "Time Duration for Run: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

}

int main(int argc, char* argv[]) {

  // this is where we put an eventual error message
  string errMsg;

  // make sure we have the arguments
  if(argc < 4) {
    std::cout << "Usage : ./TestRandomLabels managerIP managerPort whetherToPartitionData\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "whetherToPartitionData - Y yes, N no\n";
    std::cout << "whetherToRegisterLibraries - Y yes, N no\n";
  }

  //  get the manager address
  std::string ip = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  bool whetherToPartitionData = false;
  if (strcmp(argv[3], "Y")==0) {
      whetherToPartitionData = true;
  }
  bool whetherToRegisterLibraries = true;
  if (strcmp(argv[4], "N")==0) {
     whetherToRegisterLibraries = false;
  }

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, ip, clientLogger);
  PDBClient pdbClient(port, ip, clientLogger, false, true);
  
  if (whetherToRegisterLibraries) {
      pdbClient.registerType("libraries/libRedditComment.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelProjection.so", errMsg);
  }
  
  pdbClient.removeSet("redditDB", "labeledComments", errMsg);
  pdbClient.createSet<reddit::Comment>("redditDB", "labeledComments", errMsg);
  // run one iteration
  auto begin = std::chrono::high_resolution_clock::now();
  run(pdbClient);
  auto end = std::chrono::high_resolution_clock::now();
      std::cout << "End-to-End Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  // print the results
  std::cout << "Labeled Comments: \n";
  SetIterator<reddit::Comment> result = pdbClient.getSetIterator<reddit::Comment>("redditDB", "labeledComments");
  int count = 0;
  int positiveLabels = 0;
  for (const auto &r : result) {
     count++;
     if (r->label == 1){
         positiveLabels ++;
     }

  }
  std::cout << "count: " << count << std::endl;
  std::cout << "positiveLabels: " << positiveLabels << std::endl;
  pdbClient.removeSet("redditDB", "labeledComments", errMsg);

}
