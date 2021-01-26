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
        pdbClient.registerType("libraries/labelCommentsPartition.so", errMsg);
    }

    // Read the negative labeled reddit comments sets
    pdb::Handle<pdb::Computation> redditNegData = pdb::makeObject<
        ScanUserSet<reddit::Comment>>("redditDB", "negativeComments");
    pdb::Handle<pdb::Computation> partCompNeg = pdb::makeObject<LabelCommentsPartition>();
    partCompNeg->setInput(redditNegData);

    // Measure the time required for partition computation.
    auto startTimePart = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "NegativePartitionComputation", partCompNeg);
    auto endTimePart = std::chrono::high_resolution_clock::now();
    float totalSecs = std::chrono::duration_cast<
        std::chrono::duration<float>>(endTimePart - startTimePart).count();
    std::cout << "Time to run partition computation is " << totalSecs << " seconds." 
        << std::endl;
    return 0;
}
