#include <iostream>
#include <chrono>

#include "PDBClient.h"
#include "RedditComment.h"
#include "LabelCommentsPartition.h"
#include "AuthorCommentsPartition.h"
#include "SubsCommentsPartition.h"

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
        pdbClient.registerType("libraries/libAuthorCommentsPartition.so", errMsg);
        pdbClient.registerType("libraries/libSubsCommentsPartition.so", errMsg);
        pdbClient.registerType("libraries/libRedditComment.so", errMsg);
    }

    // Read the negative labeled reddit comments sets
    pdb::Handle<pdb::Computation> redditNegData = pdb::makeObject<
        ScanUserSet<reddit::Comment>>("redditDB", "negativeComments");
    pdb::Handle<pdb::Computation> partCompNeg =
        pdb::makeObject<LabelCommentsPartition>();
    partCompNeg->setInput(redditNegData);
    // Measure the time required for partition computation.
    auto startTimePart = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "NegativeLabelPartition", partCompNeg);
    auto endTimePart = std::chrono::high_resolution_clock::now();
    auto totalSecs = std::chrono::duration_cast<
        std::chrono::duration<float>>(endTimePart - startTimePart).count();
    std::cout << "Time to run label partition computation is " << totalSecs << " seconds." 
        << std::endl;

    // Overhead 1 - Partition positive set based on Authors
    pdb::Handle<pdb::Computation> redditPosData = pdb::makeObject<
        ScanUserSet<reddit::Comment>>("redditDB", "positiveComments");
    pdb::Handle<pdb::Computation> partPosAuthor = 
        pdb::makeObject<AuthorCommentsPartition>();
    partPosAuthor->setInput(redditPosData);

    auto startPosAuthor = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "PositiveAuthorPartition", partPosAuthor);
    auto endPosAuthor = std::chrono::high_resolution_clock::now();
    auto totalPosAuthor = std::chrono::duration_cast<
        std::chrono::duration<float>>(endPosAuthor - startPosAuthor).count();
    std::cout << "Overhead 1 :: Positive Partition Author :: " << totalPosAuthor << " seconds." 
        << std::endl;

    // Overhead 2 - Partition negative set based on Authors
    redditNegData = pdb::makeObject<
        ScanUserSet<reddit::Comment>>("redditDB", "negativeComments");
    pdb::Handle<pdb::Computation> partNegAuthor = 
        pdb::makeObject<AuthorCommentsPartition>();
    partNegAuthor->setInput(redditNegData);

    auto startNegAuthor = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "NegativeAuthorPartition", partNegAuthor);
    auto endNegAuthor = std::chrono::high_resolution_clock::now();
    auto totalNegAuthor = std::chrono::duration_cast<
        std::chrono::duration<float>>(endNegAuthor - startNegAuthor).count();
    std::cout << "Overhead 2 :: Negative Partition Author :: " << totalNegAuthor << " seconds." 
        << std::endl;


    // Overhead 3 - Partition negative set based on subs (subreddit_id)
    redditNegData = pdb::makeObject<
        ScanUserSet<reddit::Comment>>("redditDB", "negativeComments");
    pdb::Handle<pdb::Computation> partNegSubs = 
        pdb::makeObject<SubsCommentsPartition>();
    partNegSubs->setInput(redditNegData);

    auto startNegSubs = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "NegativeSubsPartition", partNegSubs);
    auto endNegSubs = std::chrono::high_resolution_clock::now();
    auto totalNegSubs = std::chrono::duration_cast<
        std::chrono::duration<float>>(endNegSubs - startNegSubs).count();
    std::cout << "Overhead 3 :: Negative Partition Subs :: " << totalNegSubs << " seconds." 
        << std::endl;

    return 0;
}
