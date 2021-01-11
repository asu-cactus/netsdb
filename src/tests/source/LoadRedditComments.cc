//
// by Jia, Apr 2019
//

#include "PDBClient.h"
#include "RedditComment.h"

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
  int oddHash = 0;
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
              //std::cout << Hasher<Handle<Object>>::hash(comment) << ":" << Hasher<reddit::Comment>::hash(*comment) << std::endl;
              if ((Hasher<Handle<Object>>::hash(comment) % 2) == 1){
                   oddHash++;
                   //std::cout << oddHash << ":" << comment->toString() << std::endl;
              }
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
          if ((sent >= total)&&(total !=0)) { 
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
    std::cout << "Usage : ./LoadRedditComments managerIP managerPort inputFileName whetherToPartitionData, whetherToRegisterLibraries\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "inputFileName - The file to load for reddit comments data, which is a set of JSON objects\n";
    std::cout << "whetherToPrepartitionData - Y yes, N no\n";
    std::cout << "whetherToRegisterLibraries - Y yes, N no\n";    
    std::cout << "partition on author or subs - A authors, S subs\n";
    std::cout << "whetherToRemoveSet - Y yes, N no\n";
  }
  srand (time(NULL));
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
  bool whetherToPartitionOnAuthors = true;
  if (strcmp(argv[6], "S")==0) {
      whetherToPartitionOnAuthors = false;
  }
  bool whetherToRemoveSet = true;
  if (strcmp(argv[7], "N")==0) {
      whetherToRemoveSet = false;
  }

  // make a client
  pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
  CatalogClient catalogClient(port, managerIp, clientLogger);
  PDBClient pdbClient(port, managerIp, clientLogger, false, true);
  pdbClient.registerType("libraries/libRedditComment.so", errMsg);

  // now, create a new database
  pdbClient.createDatabase("redditDB", errMsg);
  
  Handle<LambdaIdentifier> myLambda1 = nullptr;
  
  if (whetherToPartitionData) {
      if (whetherToPartitionOnAuthors) {
          //myLambda1 = makeObject<LambdaIdentifier>("reddit-a", "JoinComp_3", "attAccess_0");
          myLambda1 = makeObject<LambdaIdentifier>("reddit-three-way", "JoinComp_3", "attAccess_0");
      } else {
          //myLambda1 = makeObject<LambdaIdentifier>("reddit-s", "JoinComp_3", "attAccess_0");
          myLambda1 = makeObject<LambdaIdentifier>("reddit-three-way", "JoinComp_7", "attAccess_0");
      }
  }
  

  // now, create the output set
  if (whetherToRemoveSet == true) {
      pdbClient.removeSet("redditDB", "comments", errMsg);
  }
  pdbClient.createSet<reddit::Comment>("redditDB", "comments", errMsg, (size_t)64*(size_t)1024*(size_t)1024, "comments", nullptr, myLambda1);

  // parse the input file 
  parseInputJSONFile(pdbClient, inputFileName, 64); 

  SetIterator<reddit::Comment> result = pdbClient.getSetIterator<reddit::Comment>("redditDB", "comments");
  int count = 0;
  for (const auto &r : result) {
     count++;
  }
  std::cout << "count: " << count << std::endl;

}
