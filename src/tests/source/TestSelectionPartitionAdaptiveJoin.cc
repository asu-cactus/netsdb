#include <iostream>
#include <chrono>

#include <PDBClient.h>
#include <RedditComment.h>
#include <RedditAuthor.h>
#include <RedditSub.h>
#include <LabelCommentsPartition.h>
#include <RedditFullFeatures.h>
#include <RedditThreeWayJoin.h>
#include <ScanUserSet.h>
#include <WriteUserSet.h>

int main(int argc, char* argv[]) {
    // Create required objects
    string errMsg;
    if(argc < 2) {
        std::cout << "Usage : ./bin/redditSelectionParts managerIP managerPort\n";
        std::cout << "managerIP - IP of the manager\n";
        std::cout << "managerPort - Port of the manager\n";  
    }
    std::string managerIp = std::string(argv[1]);
    int32_t port = std::stoi(argv[2]);
    pdb::PDBLoggerPtr clientLogger = make_shared<pdb::PDBLogger>("clientLog");
    pdb::CatalogClient catalogClient(port, managerIp, clientLogger);
    pdb::PDBClient pdbClient(port, managerIp, clientLogger, false, true);

    // Read the negative labeled reddit comments sets
    pdb::Handle<pdb::Computation> redditNegData = pdb::makeObject<
        ScanUserSet<reddit::Comment>>("redditDB", "negativeComments");
    pdb::Handle<pdb::Computation> partCompNeg = pdb::makeObject<LabelCommentsPartition>();
    partCompNeg->setInput(redditNegData);

    // Measure the time required for partition computation.
    auto startTimePart = std::chrono::high_resolution_clock::now();
    pdbClient.executeComputations(errMsg, "reddit", myWriteSet, myWriteSet1);
    auto endTimePart = std::chrono::high_resolution_clock::now();
    float totalSecs = std::chrono::duration_cast<
        std::chrono::duration<float>>(endTimePart - startTimePart).count();
    std::cout << "Time to run partition computation is " << totalSecs << " seconds." 
        << std::endl;
    return 0;
}
