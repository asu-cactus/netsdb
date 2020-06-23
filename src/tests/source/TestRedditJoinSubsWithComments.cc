#include <iostream>
#include <cstring>

#include <PDBClient.h>
#include <RedditComment.h>
#include <RedditSub.h>
#include <RedditSubsAndComments.h>
#include <RedditJoinSubsAndComments.h>
#include <ScanUserSet.h>
#include <WriteUserSet.h>

using namespace pdb;


void run(PDBClient &pdbClient) {

  string errMsg;

    pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

    // clear the rankings
    pdbClient.clearSet("redditDB", "subsAndComments", "reddit::SubsAndComments", errMsg);

    // make a scan set
    Handle<Computation> input1 = makeObject<ScanUserSet<reddit::Comment>>("redditDB", "comments");
    Handle<Computation> input2 = makeObject<ScanUserSet<reddit::Sub>>("redditDB", "subs");

    // join previous ranks with links
    Handle<Computation> join = makeObject<reddit::JoinSubsWithComments>();
    join->setInput(0, input1);
    join->setInput(1, input2);


    // write it out
    Handle<Computation> myWriteSet = makeObject<WriteUserSet<reddit::SubsAndComments>>("redditDB", "subsAndComments");
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
  if(argc < 3) {

    std::cout << "Usage : ./TestRedditJoin managerIP managerPort whetherToPartitionData\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "whetherToPartitionData - Y yes, N no\n";
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
      pdbClient.registerType("libraries/libRedditSub.so", errMsg);
      pdbClient.registerType("libraries/libRedditSubsAndComments.so", errMsg);
      pdbClient.registerType("libraries/libRedditJoinSubsAndComments.so", errMsg);
  }
  
  pdbClient.removeSet("redditDB", "subsAndComments", errMsg);
  pdbClient.createSet<reddit::SubsAndComments>("redditDB", "subsAndComments", errMsg);
  // run one iteration
  auto begin = std::chrono::high_resolution_clock::now();
  run(pdbClient);
  auto end = std::chrono::high_resolution_clock::now();
      std::cout << "End-to-End Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  // print the results
  std::cout << "SubsAndComments: \n";
  SetIterator<reddit::SubsAndComments> result = pdbClient.getSetIterator<reddit::SubsAndComments>("redditDB", "subsAndComments");
  int count = 0;
  for (const auto &r : result) {
     count++;
  }
  std::cout << "count: " << count << std::endl;
  pdbClient.removeSet("redditDB", "subsAndComments", errMsg);

}
