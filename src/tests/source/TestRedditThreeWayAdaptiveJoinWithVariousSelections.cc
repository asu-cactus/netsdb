#include <iostream>
#include <cstring>

#include <PDBClient.h>
#include <RedditComment.h>
#include <RedditAuthor.h>
#include <RedditSub.h>
#include <RedditFeatures.h>
#include <RedditSubsAndComments.h>
#include <RedditJoin.h>
#include <RedditJoinSubsAndComments.h>
#include <RedditLabelSelection0_0.h>
#include <RedditLabelSelection0_1.h>
#include <RedditLabelSelection0_2.h>
#include <RedditLabelSelection0_3.h>
#include <RedditLabelSelection0_4.h>
#include <RedditLabelSelection0_5.h>
#include <RedditLabelSelection0_6.h>
#include <RedditLabelSelection0_7.h>
#include <RedditLabelSelection0_8.h>
#include <RedditLabelSelection0_9.h>
#include <RedditLabelSelection0_10.h>
#include <RedditLabelSelection1_0.h>
#include <RedditLabelSelection1_1.h>
#include <RedditLabelSelection1_2.h>
#include <RedditLabelSelection1_3.h>
#include <RedditLabelSelection1_4.h>
#include <RedditLabelSelection1_5.h>
#include <RedditLabelSelection1_6.h>
#include <RedditLabelSelection1_7.h>
#include <RedditLabelSelection1_8.h>
#include <RedditLabelSelection1_9.h>
#include <RedditLabelSelection1_10.h>
#include <ScanUserSet.h>
#include <WriteUserSet.h>

using namespace pdb;


void run(PDBClient &pdbClient, bool whetherToAdaptiveJoin, int threshold) {

  string errMsg;

    pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

    // clear the rankings
    pdbClient.clearSet("redditDB", "features", "reddit::Features", errMsg);
    pdbClient.clearSet("redditDB", "subsAndComments", "reddit::SubsAndComments", errMsg);

    // make a scan set
    Handle<Computation> input1 = makeObject<ScanUserSet<reddit::Comment>>("redditDB", "comments");
    Handle<Computation> input2 = makeObject<ScanUserSet<reddit::Author>>("redditDB", "authors");

    // make a selection
    Handle<Computation> select = nullptr;
    Handle<Computation> select1 = nullptr;

    switch (threshold) {
        case 0:
           select = makeObject<reddit::LabelSelection0_0>();
           select1 = makeObject<reddit::LabelSelection1_0>();
           break;
        case 1:
           select = makeObject<reddit::LabelSelection0_1>();
           select1 = makeObject<reddit::LabelSelection1_1>();
           break; 
        case 2:
           select = makeObject<reddit::LabelSelection0_2>();
           select1 = makeObject<reddit::LabelSelection1_2>();
           break;
        case 3:
           select = makeObject<reddit::LabelSelection0_3>();
           select1 = makeObject<reddit::LabelSelection1_3>();
           break;
        case 4:
           select = makeObject<reddit::LabelSelection0_4>();
           select1 = makeObject<reddit::LabelSelection1_4>();
           break;
        case 5:
           select = makeObject<reddit::LabelSelection0_5>();
           select1 = makeObject<reddit::LabelSelection1_5>();
           break;
        case 6:
           select = makeObject<reddit::LabelSelection0_6>();
           select1 = makeObject<reddit::LabelSelection1_6>();
           break;
        case 7:
           select = makeObject<reddit::LabelSelection0_7>();
           select1 = makeObject<reddit::LabelSelection1_7>();
           break;
        case 8:
           select = makeObject<reddit::LabelSelection0_8>();
           select1 = makeObject<reddit::LabelSelection1_8>();
           break;
        case 9:
           select = makeObject<reddit::LabelSelection0_9>();
           select1 = makeObject<reddit::LabelSelection1_9>();
           break;
        case 10:
           select = makeObject<reddit::LabelSelection0_10>();
           select1 = makeObject<reddit::LabelSelection1_10>();
           break;
        default:
           std::cout << "Fatal Error: invalid threshold value (must be an integer between 0 and 10)" << std::endl;
           exit(1);

    }
    select->setInput(input1);

    // join previous ranks with links
    Handle<Computation> join = makeObject<reddit::JoinAuthorsWithComments>();

    if (whetherToAdaptiveJoin) {
        join->setInput(0, select);
    } else {
        join->setInput(0, input1);
    }
    join->setInput(1, input2);

    // make another scan set
     Handle<Computation> input3 = makeObject<ScanUserSet<reddit::Sub>>("redditDB", "subs");

    //make another selection
    select1->setInput(input1);

    Handle<Computation> join1 = makeObject<reddit::JoinSubsWithComments>();

    if (whetherToAdaptiveJoin) {
        join1->setInput(0, select1);
    } else {
        join1->setInput(0, input1);
    }
    join1->setInput(1, input3);

    
    // write it out
    Handle<Computation> myWriteSet = makeObject<WriteUserSet<reddit::Features>>("redditDB", "features");
    myWriteSet->setInput(join);
    Handle<Computation> myWriteSet1 = makeObject<WriteUserSet<reddit::SubsAndComments>>("redditDB", "subsAndComments");
    myWriteSet1->setInput(join1);


    // execute the computation
    auto begin = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "reddit-three-way-"+std::to_string(threshold), myWriteSet, myWriteSet1);
    auto end = std::chrono::high_resolution_clock::now();
          std::cout << "Time Duration for Run: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

}

int main(int argc, char* argv[]) {

  // this is where we put an eventual error message
  string errMsg;

  // make sure we have the arguments
  if(argc < 5) {
    std::cout << "Usage : ./TestRedditAdaptiveThreeWayJoinWithVariousSelections managerIP managerPort whetherToAdaptiveJoin whetherToRegisterLibraries threshold\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "whetherToAdaptiveJoin - Y yes, N no\n";
    std::cout << "whetherToRegisterLibraries - Y yes, N no\n";
    std::cout << "threshold - an integer between 0 and 10\n";
  }

  //  get the manager address
  std::string ip = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  bool whetherToAdaptiveJoin = false;
  if (strcmp(argv[3], "Y")==0) {
      whetherToAdaptiveJoin = true;
  }
  bool whetherToRegisterLibraries = true;
  if (strcmp(argv[4], "N")==0) {
     whetherToRegisterLibraries = false;
  }

  int selectionThreshold = atoi(argv[5]);

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, ip, clientLogger);
  PDBClient pdbClient(port, ip, clientLogger, false, true);
  
  if (whetherToRegisterLibraries) {
      pdbClient.registerType("libraries/libRedditComment.so", errMsg);
      pdbClient.registerType("libraries/libRedditAuthor.so", errMsg);
      pdbClient.registerType("libraries/libRedditSub.so", errMsg);
      pdbClient.registerType("libraries/libRedditFeatures.so", errMsg);
      pdbClient.registerType("libraries/libRedditSubsAndComments.so", errMsg);
      pdbClient.registerType("libraries/libRedditJoin.so", errMsg);
      pdbClient.registerType("libraries/libRedditJoinSubsAndComments.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_0.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_1.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_2.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_3.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_4.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_5.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_6.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_7.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_8.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_9.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection0_10.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_0.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_1.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_2.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_3.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_4.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_5.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_6.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_7.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_8.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_9.so", errMsg);
      pdbClient.registerType("libraries/libRedditLabelSelection1_10.so", errMsg);
  }
  
  pdbClient.removeSet("redditDB", "features", errMsg);
  pdbClient.createSet<reddit::Features>("redditDB", "features", errMsg);
  pdbClient.removeSet("redditDB", "subsAndComments", errMsg);
  pdbClient.createSet<reddit::Features>("redditDB", "subsAndComments", errMsg);
  // run one iteration
  auto begin = std::chrono::high_resolution_clock::now();
  run(pdbClient, whetherToAdaptiveJoin, selectionThreshold);
  auto end = std::chrono::high_resolution_clock::now();
      std::cout << "End-to-End Time Duration: "
              << std::chrono::duration_cast<std::chrono::duration<float>>(end - begin).count()
              << " secs." << std::endl;

  // print the results
  std::cout << "Features: \n";
  SetIterator<reddit::Features> result1 = pdbClient.getSetIterator<reddit::Features>("redditDB", "features");
  int count = 0;
  for (const auto &r : result1) {
     count++;
  }
  std::cout << "count: " << count << std::endl;
  pdbClient.removeSet("redditDB", "features", errMsg);

  std::cout << "SubsAndComments: \n";
  SetIterator<reddit::SubsAndComments> result2 = pdbClient.getSetIterator<reddit::SubsAndComments>("redditDB", "subsAndComments");
  count = 0;
  for (const auto &r : result2) {
     count++;
  }
  std::cout << "count: " << count << std::endl;
  pdbClient.removeSet("redditDB", "subsAndComments", errMsg);

  std::cout << "Comments: \n";
  SetIterator<reddit::Comment> result3 = pdbClient.getSetIterator<reddit::Comment>("redditDB", "comments");
  int positiveCount = 0;
  int negativeCount = 0;
  count = 0;
  for (const auto &r : result3) {
     count++;
     if (r->label < selectionThreshold)
         positiveCount++;
     else
         negativeCount++;   
  }
  std::cout << "Total " << count << " comments" << std::endl;
  std::cout << "Total " << positiveCount << " positive comments" << std::endl;
  std::cout << "Total " << negativeCount << " negative comments" << std::endl;
}
