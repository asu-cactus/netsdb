#include <iostream>
#include <cstring>

#include <PDBClient.h>
#include <URLRankMultiSelection.h>
#include <DistinctProjection.h>
#include <Link.h>
#include <LinkScanner.h>
#include <RankedUrlWriter.h>
#include <RankedUrlScanner.h>
#include <JoinRankedUrlWithLink.h>
#include <RankUpdateAggregation.h>
#include <LinkWithValue.h>
#include <URLURLsRank.h>

using namespace pdb;

void initRankings(PDBClient &pdbClient) {
  
  // put the error
  string errMsg;

  // we put our graph here
  pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

  // make a scan set
  Handle<Computation> input = makeObject<LinkScanner>("db", "links");


  // make the projection query
  Handle<Computation> projectionQuery = makeObject<DistinctProjection>();
  projectionQuery->setInput(input);

  // writes out the rankings
  Handle<Computation> myWriteSet = makeObject<RankedUrlWriter>("db", "rankings_0");
  myWriteSet->setInput(projectionQuery);

  // execute the computation
  auto begin = std::chrono::high_resolution_clock::now();
  pdbClient.executeComputations(errMsg, "initRankings", myWriteSet);
  auto end = std::chrono::high_resolution_clock::now();
      std::cout << "Time Duration for initRankings: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
}


void run(PDBClient &pdbClient, uint64_t numIter) {

  string errMsg;
  for(uint64_t i = 0; i < numIter; ++i) {

    pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

    // clear the rankings
    pdbClient.clearSet("db", "rankings_" + std::to_string(1 - (i % 2)), "pdb::RankedUrl", errMsg);

    // make a scan set
    Handle<Computation> input1 = makeObject<LinkScanner>("db", "links");
    Handle<RankedUrlScanner> input2 = makeObject<RankedUrlScanner>("db", "rankings_" + std::to_string(i % 2));

    // join previous ranks with links
    Handle<Computation> join1 = makeObject<JoinRankedUrlWithLink>();
    join1->setInput(0, input1);
    join1->setInput(1, input2);

    // join the joined result with counts
    Handle<Computation> multiSelection = makeObject<URLRankMultiSelection>();
    multiSelection->setInput(0, join1);

    // aggregate them
    Handle<RankUpdateAggregation> agg = makeObject<RankUpdateAggregation>();
    agg->setInput(multiSelection);

    // write it out
    Handle<RankedUrlWriter> myWriteSet = makeObject<RankedUrlWriter>("db", "rankings_" + std::to_string(1 - (i % 2)));
    myWriteSet->setInput(agg);

    // execute the computation
    auto begin = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "pageRankIteration_"+std::to_string(1 - (i % 2)), myWriteSet);
    auto end = std::chrono::high_resolution_clock::now();
          std::cout << "Time Duration for Run: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;
  }

}

int main(int argc, char* argv[]) {

  // this is where we put an eventual error message
  string errMsg;

  // make sure we have the arguments
  if(argc < 4) {

    std::cout << "Usage : ./TestSparseMultiply managerIP managerPort numIter whetherToPartitionData\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "numIter - The nubmer of iterations to run\n";
    std::cout << "whetherToPartitionData - Y yes, N no\n";
  }

  //  get the manager address
  std::string ip = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  int32_t numIter = std::stoi(argv[3]);
  bool whetherToPartitionData = false;
  if (strcmp(argv[4], "Y")==0) {
      whetherToPartitionData = true;
  }
  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, ip, clientLogger);
  PDBClient pdbClient(port, ip, clientLogger, false, true);
  

  // run one iteration
  auto begin = std::chrono::high_resolution_clock::now();
  run(pdbClient, numIter);
  auto end = std::chrono::high_resolution_clock::now();
      std::cout << "End-to-End Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  // print the results
  std::cout << "Rankings: \n";
  SetIterator<RankedUrl> result = pdbClient.getSetIterator<RankedUrl>("db", "rankings_1");
  /*for (const auto &r : result) {
    std::cout << r->url << ", " << r->rank << '\n';
    std::cout << "\n";
  }
  */
  pdbClient.removeSet("db", "rankings_0", errMsg);
  pdbClient.removeSet("db", "rankings_1", errMsg);

}
