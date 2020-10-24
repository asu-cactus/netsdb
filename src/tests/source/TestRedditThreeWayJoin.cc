#include <iostream>
#include <cstring>

#include <PDBClient.h>
#include <RedditComment.h>
#include <RedditAuthor.h>
#include <RedditSub.h>
#include <RedditFullFeatures.h>
#include <RedditThreeWayJoin.h>
#include <ScanUserSet.h>
#include <WriteUserSet.h>

using namespace pdb;


void run(PDBClient &pdbClient) {

  string errMsg;

    pdb::makeObjectAllocatorBlock(128 * 1024 * 1024, true);

    // clear the rankings
    pdbClient.clearSet("redditDB", "fullfeatures", "reddit::FullFeatures", errMsg);

    // make a scan set
    Handle<Computation> input1 = makeObject<ScanUserSet<reddit::Comment>>("redditDB", "comments");
    Handle<Computation> input2 = makeObject<ScanUserSet<reddit::Author>>("redditDB", "authors");
    Handle<Computation> input3 = makeObject<ScanUserSet<reddit::Sub>>("redditDB", "subs");

    // join previous ranks with links
    Handle<Computation> join = makeObject<reddit::ThreeWayJoin>();
    join->setInput(0, input1);
    join->setInput(1, input2);
    join->setInput(2, input3);

    // write it out
    Handle<Computation> myWriteSet = makeObject<WriteUserSet<reddit::FullFeatures>>("redditDB", "fullfeatures");
    myWriteSet->setInput(join);

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

    std::cout << "Usage : ./TestRedditThreeWayJoin managerIP managerPort whetherToPartitionData\n";
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
      pdbClient.registerType("libraries/libRedditAuthor.so", errMsg);
      pdbClient.registerType("libraries/libRedditSub.so", errMsg);
      pdbClient.registerType("libraries/libRedditFullFeatures.so", errMsg);
      pdbClient.registerType("libraries/libRedditThreeWayJoin.so", errMsg);
  }
  
  pdbClient.removeSet("redditDB", "fullfeatures", errMsg);
  pdbClient.createSet<reddit::FullFeatures>("redditDB", "fullfeatures", errMsg);
  // run one iteration
  auto begin = std::chrono::high_resolution_clock::now();
  run(pdbClient);
  auto end = std::chrono::high_resolution_clock::now();
      std::cout << "End-to-End Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  // print the results
  std::cout << "FullFeatures: \n";
  SetIterator<reddit::FullFeatures> result = pdbClient.getSetIterator<reddit::FullFeatures>("redditDB", "fullfeatures");
  int count = 0;
  for (const auto &r : result) {
     count++;
  }
  std::cout << "count: " << count << std::endl;
  pdbClient.removeSet("redditDB", "fullfeatures", errMsg);

}
