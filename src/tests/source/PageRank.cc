#include <iostream>
#include <cstring>

#include <PDBClient.h>
#include <URLRankMultiSelection.h>
#include <DistinctProjection.h>
#include <Link.h>
#include <LinkScanner.h>
#include <RankedUrlWriter.h>
#include <RankedUrlScanner.h>
#include <LinkPartition.h>
#include <RankPartition.h>
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

void repartition(PDBClient &pdbClient) {
      Handle<LambdaIdentifier> myLambda1 = nullptr;
      myLambda1 = makeObject<LambdaIdentifier>("pageRankIteration_1", "JoinComp_2", "attAccess_1");
      std::string errMsg;

      // now, create the output set
      pdbClient.removeSet("db", "repartitionedRankings_0", errMsg);
      pdbClient.createSet<RankedUrl>("db", "repartitionedRankings_0", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "rankings_0", nullptr, myLambda1);
      pdbClient.removeSet("db", "repartitionedRankings_1", errMsg);
      pdbClient.createSet<RankedUrl>("db", "repartitionedRankings_1", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "rankings_1", nullptr, myLambda1);

      // n w, create the  utput set
      //
      pdbClient.removeSet("db", "repartitionedLinks", errMsg);
      Handle<LambdaIdentifier> myLambda = nullptr;
      myLambda = makeObject<LambdaIdentifier>("pageRankIteration_1", "JoinComp_2", "attAccess_0");
      pdbClient.createSet<Link>("db", "repartitionedLinks", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "loadLinks", nullptr, myLambda);

      //repartition Links
      Handle<Computation> input1 = makeObject<LinkScanner>("db", "links");
      Handle<LinkPartition> linkPartition = makeObject<LinkPartition>("db", "repartitionedLinks");
      linkPartition->setInput(input1);
      pdbClient.executeComputations(errMsg, "link_partitions", linkPartition);

      //repartition Rank1
      Handle<Computation> input2 = makeObject<RankedUrlScanner>("db", "rankings_0");
      Handle<RankPartition> rankPartition2 = makeObject<RankPartition>("db", "repartitionedRankings_0");
      rankPartition2->setInput(input2);
      pdbClient.executeComputations(errMsg, "rank0_partitions", rankPartition2);

      //repartition Rank2
      Handle<Computation> input3 = makeObject<RankedUrlScanner>("db", "rankings_1");
      Handle<RankPartition> rankPartition3 = makeObject<RankPartition>("db", "repartitionedRankings_1");
      rankPartition3->setInput(input3);
      pdbClient.executeComputations(errMsg, "rank1_partitions", rankPartition3);

     /* SetIterator<RankedUrl> result1 =
            pdbClient.getSetIterator<RankedUrl>("db", "rankings_0");

      int count = 0;
      for (auto a : result1)
          count ++;
      std::cout << "Rankings_0:" << count << std::endl;


      SetIterator<RankedUrl> partitionedResult1 =
            pdbClient.getSetIterator<RankedUrl>("db", "repartitionedRankings_0");

      count = 0;
    
      for (auto a : partitionedResult1)
          count ++;
      std::cout << "repartitionedRankings_0:" << count << std::endl;

      SetIterator<RankedUrl> result2 =
            pdbClient.getSetIterator<RankedUrl>("db", "rankings_1");

      count = 0;
      for (auto a : result2)
          count ++;
      std::cout << "Rankings_1:" << count << std::endl;

      SetIterator<RankedUrl> partitionedResult2 =
            pdbClient.getSetIterator<RankedUrl>("db", "repartitionedRankings_1");

      count = 0;

      for (auto a : partitionedResult2)
          count ++;
      std::cout << "repartitionedRankings_1:" << count << std::endl;


      SetIterator<Link> result3 =
            pdbClient.getSetIterator<Link>("db", "links");

      count = 0;
      for (auto a : result3)
          count ++;
      std::cout << "links:" << count << std::endl;
      
      SetIterator<Link> partitionedResult3 =
            pdbClient.getSetIterator<Link>("db", "repartitionedLinks");

      count = 0;

      for (auto a : partitionedResult3)
          count ++;
      std::cout << "repartitionedLinks:" << count << std::endl;
     */

}



void run(PDBClient &pdbClient, uint64_t numIter, bool repartitioned, int index) {

  string errMsg;
  for(uint64_t i = index; i < numIter; ++i) {

    pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

    // clear the rankings
    if (!repartitioned)
        pdbClient.clearSet("db", "rankings_" + std::to_string(1 - (i % 2)), "pdb::RankedUrl", errMsg);
    else 
        pdbClient.clearSet("db", "repartitionedRankings_" + std::to_string(1 - (i % 2)), "pdb::RankedUrl", errMsg);
    // make a scan set
    Handle<Computation> input1 = nullptr;
    Handle<RankedUrlScanner> input2 = nullptr;
    if (!repartitioned) {
        input1 = makeObject<LinkScanner>("db", "links");
        input2 = makeObject<RankedUrlScanner>("db", "rankings_" + std::to_string(i % 2));
    }
    else {
        input1 = makeObject<LinkScanner>("db", "repartitionedLinks");
        input2 = makeObject<RankedUrlScanner>("db", "repartitionedRankings_" + std::to_string(i % 2));
    }
    // join previous ranks with links
    Handle<Computation> join1 = makeObject<JoinRankedUrlWithLink>();
    join1->setInput(0, input1);
    join1->setInput(1, input2);

    // join the joined result with counts
    Handle<Computation> multiSelection = makeObject<URLRankMultiSelection>();
    multiSelection->setInput(0, join1);

    // aggregate them
    Handle<RankUpdateAggregation> agg = nullptr;
    if (!repartitioned)
        agg = makeObject<RankUpdateAggregation>("db", "rankings_" + std::to_string(1 - (i % 2)));
    else
        agg = makeObject<RankUpdateAggregation>("db", "repartitionedRankings_" + std::to_string(1 - (i % 2)));
    agg->setInput(multiSelection);

    // write it out
    //Handle<RankedUrlWriter> myWriteSet = makeObject<RankedUrlWriter>("db", "rankings_" + std::to_string(1 - (i % 2)));
    //myWriteSet->setInput(agg);

    // execute the computation
    auto begin = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "pageRankIteration_"+std::to_string(1 - (i % 2)), agg);
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
  if(argc < 5) {

    std::cout << "Usage : ./TestSparseMultiply managerIP managerPort numIter whetherToPartitionData\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "numIter - The nubmer of iterations to run\n";
    std::cout << "whetherToPartitionData - Y yes, N no\n";
    std::cout << "reactive - Y yes, N no\n";

  }

  //  get the manager address
  std::string ip = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  int32_t numIter = std::stoi(argv[3]);
  bool whetherToPartitionData = false;
  if (strcmp(argv[4], "Y")==0) {
      whetherToPartitionData = true;
  }
  bool reactive = false;
  if (strcmp(argv[5], "Y")==0) {
      reactive = true;
  }
  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, ip, clientLogger);
  PDBClient pdbClient(port, ip, clientLogger, false, true);
  

  // run one iteration
  auto begin = std::chrono::high_resolution_clock::now();
  if (!reactive) {
      run(pdbClient, numIter, false, 0);
  } else {
      run(pdbClient, 1, false, 0);
      repartition(pdbClient);
      run(pdbClient, numIter-1, true, 1);
  }
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
  //pdbClient.removeSet("db", "rankings_0", errMsg);
  //pdbClient.removeSet("db", "rankings_1", errMsg);

}
