#include <iostream>
#include <chrono>

#include "PDBClient.h"
#include "RedditComment.h"
#include "RedditAuthor.h"
#include "RedditSub.h"
#include "LabelCommentsPartition.h"
#include "RedditFullFeatures.h"
#include "RedditThreeWayJoin.h"

#include <ScanUserSet.h>
#include <WriteUserSet.h>

int main(int argc, char* argv[]) {
    // Create required objects
    string errMsg;
    if(argc < 3) {
        std::cout << "Usage : ./bin/redditSelectionParts managerIP managerPort\n";
        std::cout << "managerIP - IP of the manager\n";
        std::cout << "managerPort - Port of the manager\n";  
        std::cout << "whetherToRegisterLibraries - Y yes, N no\n";
    }
    bool whetherToRegisterLibraries = true;
    if (strcmp(argv[3], "N")==0) {
        whetherToRegisterLibraries = false;
    }
    std::string managerIp = std::string(argv[1]);
    int32_t port = std::stoi(argv[2]);
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
    pdb::CatalogClient catalogClient(port, managerIp, clientLogger);
    pdb::PDBClient pdbClient(port, managerIp, clientLogger, false, true);

    // Register the required objects
    if (whetherToRegisterLibraries) {
        pdbClient.registerType("libraries/libLabelCommentsPartition.so", errMsg);
        pdbClient.registerType("libraries/libRedditComment.so", errMsg);
        pdbClient.registerType("libraries/libRedditAuthor.so", errMsg);
        pdbClient.registerType("libraries/libRedditSub.so", errMsg);
        pdbClient.registerType("libraries/libRedditFullFeatures.so", errMsg);
        pdbClient.registerType("libraries/libRedditThreeWayJoin.so", errMsg);
    }

    // Read the negative labeled reddit comments sets
    pdb::Handle<pdb::Computation> redditNegData = pdb::makeObject<
        ScanUserSet<reddit::Comment>>("redditDB", "negativeComments");
    pdb::Handle<pdb::Computation> partCompNeg =
        pdb::makeObject<LabelCommentsPartition>("redditDB", "negativeComments");
    partCompNeg->setInput(redditNegData);

    // Measure the time required for partition computation.
    auto startTimePart = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "NegativePartitionComputation", partCompNeg);
    auto endTimePart = std::chrono::high_resolution_clock::now();
    float totalSecs = std::chrono::duration_cast<
        std::chrono::duration<float>>(endTimePart - startTimePart).count();
    std::cout << "Time to run partition computation is " << totalSecs << " seconds." 
        << std::endl;

    // Three way join between Negative Comments, Authors, Subreddit
    pdbClient.removeSet("redditDB", "fullfeatures", errMsg);
    pdbClient.createSet<reddit::FullFeatures>("redditDB", "fullfeatures", errMsg);

    pdb::Handle<pdb::Computation> redditNegDataset = pdb::makeObject<
        ScanUserSet<reddit::Comment>>("redditDB", "negativeComments");
    pdb::Handle<pdb::Computation> authorDataset = pdb::makeObject<
        ScanUserSet<reddit::Author>>("redditDB", "authors");
    pdb::Handle<pdb::Computation> subsDataset = pdb::makeObject<
        ScanUserSet<reddit::Author>>("redditDB", "subs");
    pdb::Handle<pdb::Computation> opWriteSet = pdb::makeObject<
        WriteUserSet<reddit::FullFeatures>>("redditDB", "fullfeatures");
    pdb::Handle<pdb::Computation> threeWayJoin = pdb::makeObject<reddit::ThreeWayJoin>();
    threeWayJoin->setInput(0, redditNegDataset);
    threeWayJoin->setInput(1, authorDataset);
    threeWayJoin->setInput(2, subsDataset);
    opWriteSet->setInput(threeWayJoin)

    // Measure the time required for the Three Way Join
    auto startTimeJoin = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "NegativeThreeWayJoin", opWriteSet);
    auto endTimeJoin = std::chrono::high_resolution_clock::now();
    float totalJoinSecs = std::chrono::duration_cast<
        std::chrono::duration<float>>(endTimeJoin - startTimeJoin).count();
    std::cout << "Time to run negative three way join is " << totalJoinSecs << " seconds." 
        << std::endl;

    return 0;
}
