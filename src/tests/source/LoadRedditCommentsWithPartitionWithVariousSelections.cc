//
// by Jia, Apr 2019
//

#include "PDBClient.h"
#include "RedditComment.h"
#include <RedditAuthor.h>
#include <RedditSub.h>
#include <RedditFeatures.h>
#include <RedditSubsAndComments.h>
#include <RedditJoin.h>
#include <RedditJoinSubsAndComments.h>
#include <RedditPositiveLabelSelection.h>
#include <RedditNegativeLabelSelection.h>
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
#include <string>
#include <random>
#include <chrono>

using namespace pdb;

static void classify_v1(pdb::Handle<reddit::Comment> comment){
   int sampled_variable = rand()%10;
   comment->label = sampled_variable;
}


static void classify(pdb::Handle<reddit::Comment> comment){
   if (comment->score > 1) {
       comment->label = 1;
   } else{
       comment->label = -1;
   }
}

void parseInputJSONFile(PDBClient &pdbClient, std::string fileName, int blockSizeInMB) {

  // the error message is put there
  string errMsg;
  std ::ifstream inFile(fileName.c_str());
  std :: string line;
  bool end = false;
  bool rollback = false;

  long total = 10000000;
  long sent = 0;
  long i = 0;
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
          pdb::Handle<reddit::Comment> comment = pdb::makeObject<reddit::Comment>(i, line);
          classify_v1(comment);
          if (strcmp(comment->author.c_str(), "[deleted]") !=0){
              storeMe->push_back(comment);
              i++;
          }
      }
      catch (pdb::NotEnoughSpace &n) {
          if (! pdbClient.sendData<reddit::Comment> (std::pair<std::string, std::string>("comments", "redditDB"), storeMe, errMsg)) {
             std::cout << "Failed to send data to dispatcher server" << std::endl;
             return;
          }
          std::cout << "Dispatched " << storeMe->size() << " comments." << std::endl;
          sent = sent+storeMe->size();
          std::cout << "sent " << sent << " objects in total" << std::endl;
          if ((sent >= total)&&(total!=0)) { 
              end = true;
          }
          rollback = true; 
          pdb::makeObjectAllocatorBlock((size_t)blockSizeInMB * (size_t)1024 * (size_t)1024, true);
          storeMe = pdb::makeObject<pdb::Vector<pdb::Handle<reddit::Comment>>> ();

      }
   }
   pdbClient.flushData (errMsg);
}



int main(int argc, char* argv[]) {
 
  // we put the error here
  string errMsg;

  // make sure we have the arguments
  if(argc < 6) {

    std::cout << "Usage : ./LoadRedditCommentsWithPartition managerIP managerPort inputFileName whetherToPartitionData, whetherToRegisterLibraries\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "inputFileName - The file to load for reddit comments data, which is a set of JSON objects\n";
    std::cout << "whetherToPrepartitionData - Y yes, N no\n";
    std::cout << "whetherToRegisterLibraries - Y yes, N no\n";    
    std::cout << "threshold - an integer number between 0 and 10\n";
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
  int threshold = atoi(argv[6]);

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, managerIp, clientLogger);
  PDBClient pdbClient(port, managerIp, clientLogger, false, true);
  pdbClient.registerType("libraries/libRedditComment.so", errMsg);
  pdbClient.registerType("libraries/libRedditAuthor.so", errMsg);
  pdbClient.registerType("libraries/libRedditSub.so", errMsg);
  pdbClient.registerType("libraries/libRedditFeatures.so", errMsg);
  pdbClient.registerType("libraries/libRedditSubsAndComments.so", errMsg);
  pdbClient.registerType("libraries/libRedditJoin.so", errMsg);
  pdbClient.registerType("libraries/libRedditJoinSubsAndComments.so", errMsg);
  pdbClient.registerType("libraries/libRedditPositiveLabelSelection.so", errMsg);
  pdbClient.registerType("libraries/libRedditNegativeLabelSelection.so", errMsg);
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

  // now, create a new database
  pdbClient.createDatabase("redditDB", errMsg);
  
  Handle<LambdaIdentifier> myLambda1 = nullptr;

  ///construct dispatching computations
  pdb::makeObjectAllocatorBlock(64 * 1024 * 1024, true);

  // make a scan set
  Handle<Computation> input1 = makeObject<ScanUserSet<reddit::Comment>>("redditDB", "comments");
  Handle<Computation> input2 = makeObject<ScanUserSet<reddit::Author>>("redditDB", "authors");

  // make a selection
  Handle<Computation> select = nullptr;
  Handle<Computation> select1 = nullptr;

  switch (threshold) {
        case 0:
           select= makeObject<reddit::LabelSelection0_0>();
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

  join->setInput(0, select);
  join->setInput(1, input2);

  // make another scan set
  Handle<Computation> input3 = makeObject<ScanUserSet<reddit::Sub>>("redditDB", "subs");

  //make another selection
  select1->setInput(input1);

  Handle<Computation> join1 = makeObject<reddit::JoinSubsWithComments>();

  join1->setInput(0, select1);
  join1->setInput(1, input3);


  // write it out
  Handle<Computation> myWriteSet = makeObject<WriteUserSet<reddit::Features>>("redditDB", "features");
  myWriteSet->setInput(join);
  Handle<Computation> myWriteSet1 = makeObject<WriteUserSet<reddit::SubsAndComments>>("redditDB", "subsAndComments");
  myWriteSet1->setInput(join1);

  Handle<Vector<Handle<Computation>>> sinks = makeObject<Vector<Handle<Computation>>>();
  sinks->push_back(myWriteSet);
  sinks->push_back(myWriteSet1);
 

  Handle<Vector<Handle<Computation>>> computations = makeObject<Vector<Handle<Computation>>>();
  computations->push_back(myWriteSet);
  computations->push_back(myWriteSet1);
  computations->push_back(join);
  computations->push_back(join1);
  computations->push_back(select);
  computations->push_back(select1);
  computations->push_back(input1);
  computations->push_back(input2);
  computations->push_back(input3); 

  // now, create the output set
  pdbClient.removeSet("redditDB", "comments", errMsg);

  if (whetherToPartitionData) {
      pdbClient.createSet<reddit::Comment>("redditDB", "comments", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "comments", computations, "reddit-three-way"+std::to_string(threshold));
  } else {
      pdbClient.createSet<reddit::Comment>("redditDB", "comments", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "comments");
  }

  // parse the input file 
  parseInputJSONFile(pdbClient, inputFileName, 64); 

}
