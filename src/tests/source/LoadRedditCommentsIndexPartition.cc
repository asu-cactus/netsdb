#include "CommentPartition.h"
#include "PDBClient.h"
#include "RedditComment.h"
#include <RedditAuthor.h>
#include <RedditFeatures.h>
#include <RedditJoin.h>
#include <RedditJoinSubsAndComments.h>
#include <RedditNegativeLabelSelection.h>
#include <RedditPositiveLabelSelection.h>
#include <RedditSub.h>
#include <RedditSubsAndComments.h>
#include <ScanUserSet.h>
#include <WriteUserSet.h>
#include <chrono>
#include <random>
#include <string>

using namespace pdb;

void parseInputJSONFile(PDBClient &pdbClient, std::string fileName,
                        int blockSizeInMB, std::string setName,
                        std::string dbName) {

  // the error message is put there
  string errMsg;
  std ::ifstream inFile(fileName.c_str());
  std ::string line;
  bool end = false;
  bool rollback = false;

  long total = 10000000;
  long sent = 0;
  long i = 0;
  int oddHash = 0;
  makeObjectAllocatorBlock((size_t)blockSizeInMB * (size_t)1024 * (size_t)1024,
                           true);
  Handle<Vector<Handle<reddit::Comment>>> storeMe =
      makeObject<Vector<Handle<reddit::Comment>>>();
  while (!end) {
    if (!rollback) {
      if (!std::getline(inFile, line)) {
        end = true;
        if (!pdbClient.sendData<reddit::Comment>(
                std::pair<std::string, std::string>(setName, dbName), storeMe,
                errMsg)) {
          std::cout << "Failed to send data to dispatcher server" << std::endl;
          return;
        }
        pdbClient.flushData(errMsg);
        std::cout << "Dispatched " << storeMe->size() << " comments."
                  << std::endl;
        break;
      }
    }
    rollback = false;
    try {
      Handle<reddit::Comment> comment = makeObject<reddit::Comment>(i, line);
      if (strcmp(comment->author.c_str(), "[deleted]") != 0) {
        comment->index = i;
        storeMe->push_back(comment);
        i++;
      }
    } catch (NotEnoughSpace &n) {
      if (!pdbClient.sendData<reddit::Comment>(
              std::pair<std::string, std::string>(setName, dbName), storeMe,
              errMsg)) {
        std::cout << "Failed to send data to dispatcher server" << std::endl;
        return;
      }
      std::cout << "Dispatched " << storeMe->size() << " comments."
                << std::endl;
      sent = sent + storeMe->size();
      std::cout << "sent " << sent << " objects in total" << std::endl;
      if ((sent >= total) && (total != 0)) {
        end = true;
      }
      rollback = true;
      makeObjectAllocatorBlock(
          (size_t)blockSizeInMB * (size_t)1024 * (size_t)1024, true);
      storeMe = makeObject<Vector<Handle<reddit::Comment>>>();
    }
  }
  pdbClient.flushData(errMsg);
}

int main(int argc, char *argv[]) {

  // we put the error here
  string errMsg;

  // make sure we have the arguments
  if (argc < 6) {

    std::cout << "Usage : ./LoadRedditCommentsWithPartition managerIP "
                 "managerPort inputFileName runComputation deleteData\n";
    std::cout << "managerIP - IP of the manager\n";
    std::cout << "managerPort - Port of the manager\n";
    std::cout << "inputFileName - The file to load for reddit comments data, "
                 "which is a set of JSON objects\n";
    std::cout << "runComputation\n";
    std::cout << "deleteData\n";
  }

  //  get the manager address
  std::string managerIp = std::string(argv[1]);
  int32_t port = std::stoi(argv[2]);
  std::string inputFileName = std::string(argv[3]);
  bool runComputation = strcmp(argv[4], "Y") == 0;
  bool deleteData = strcmp(argv[5], "Y") == 0;

  // make a client
  PDBLoggerPtr clientLogger = make_shared<PDBLogger>("clientLog");
  CatalogClient catalogClient(port, managerIp, clientLogger);
  PDBClient pdbClient(port, managerIp, clientLogger, false, true);
  pdbClient.registerType("libraries/libRedditComment.so", errMsg);
  pdbClient.registerType("libraries/libRedditCommentPartition.so", errMsg);

  if (deleteData) {
    pdbClient.deleteSet("redditDB", "comments_unpart");
    pdbClient.deleteSet("redditDB", "comments");
  }

  // now, create a new database
  pdbClient.createDatabase("redditDB", errMsg);

  {
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 256};

    if (runComputation) {
      pdbClient.createSet<reddit::Comment>(
          "redditDB", "comments_unpart", errMsg,
          (size_t)128 * (size_t)1024 * (size_t)1024, "comments");
      pdbClient.createSet<reddit::Comment>(
          "redditDB", "comments", errMsg,
          (size_t)128 * (size_t)1024 * (size_t)1024, "comments");

      parseInputJSONFile(pdbClient, inputFileName, 128, "comments_unpart",
                         "redditDB");

      // make the computation
      pdb::Handle<pdb::Computation> readA =
          makeObject<ScanUserSet<reddit::Comment>>("redditDB",
                                                   "comments_unpart");

      pdb::Handle<pdb::Computation> sel =
          pdb::makeObject<reddit::CommentPartition>("redditDB", "comments");
      sel->setInput(readA);

      // run the computation
      if (!pdbClient.executeComputations(errMsg, "reddit-comments-index",
                                         sel)) {
        cout << "Computation failed. Message was: " << errMsg << "\n";
        exit(1);
      }
    } else {
      Handle<LambdaIdentifier> myLambda1 = makeObject<LambdaIdentifier>(
          "reddit-comments-index", "PartitionComp_1", "native_lambda_0");
      pdbClient.createSet<reddit::Comment>("redditDB", "comments", errMsg,
                                           (size_t)64 * (size_t)1024 *
                                               (size_t)1024,
                                           "comments", nullptr, myLambda1);
      parseInputJSONFile(pdbClient, inputFileName, 128, "comments", "redditDB");
    }
  }
}
