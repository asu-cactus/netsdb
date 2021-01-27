#include <PDBClient.h>
#include <RedditComment.h>
#include <RedditPositiveLabelSelection.h>
#include <RedditNegativeLabelSelection.h>
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
    // Register the required reddit comment structure in the PDB Catalog
    pdbClient.registerType("libraries/libRedditComment.so", errMsg);
    pdbClient.registerType("libraries/libRedditPositiveLabelSelection.so", errMsg);
    pdbClient.registerType("libraries/libRedditNegativeLabelSelection.so", errMsg);

    // Create/Delete required sets
    pdbClient.removeSet("redditDB", "negativeComments", errMsg);
    pdbClient.removeSet("redditDB", "positiveComments", errMsg);
    const pdb::UseTemporaryAllocationBlock tempBlock{1024 * 1024 * 128};
    pdbClient.createSet<reddit::Comment>("redditDB", "negativeComments", errMsg,
        (size_t)64*(size_t)1024*(size_t)1024, "negativeComments", nullptr, nullptr);
    pdbClient.createSet<reddit::Comment>("redditDB", "positiveComments", errMsg,
        (size_t)64*(size_t)1024*(size_t)1024, "positiveComments", nullptr, nullptr);
    // Read the reddits comments data
    pdb::Handle<pdb::Computation> redditData = pdb::makeObject<
        ScanUserSet<reddit::Comment>>("redditDB", "comments");

    // Selection Computation
    pdb::Handle<pdb::Computation> negativeLabel =
        pdb::makeObject<reddit::NegativeLabelSelection>();
    pdb::Handle<pdb::Computation> positiveLabel =
        pdb::makeObject<reddit::PositiveLabelSelection>();
    negativeLabel->setInput(redditData);
    positiveLabel->setInput(redditData);

    // Write the selected rows to respective sets
    pdb::Handle<pdb::Computation> writeNegativeSet = pdb::makeObject<
        WriteUserSet<reddit::Comment>>("redditDB", "negativeComments");
    pdb::Handle<pdb::Computation> writePositiveSet = pdb::makeObject<
        WriteUserSet<reddit::Comment>>("redditDB", "positiveComments");
    writeNegativeSet->setInput(negativeLabel);
    writePositiveSet->setInput(positiveLabel);
    // Execute write operation for both the set
    pdbClient.executeComputations(errMsg, "RedditSelectionParts",
        writeNegativeSet, writePositiveSet);
    // Count the number of rows in both the sets
    // FIXME The count operation for the sets gives error
    // FIXME Problem forwarding data to client: PDBCommunicator: not able to send the object type
    // SetIterator<reddit::Comment> negativeIter =
    //     pdbClient.getSetIterator<reddit::Comment>("redditDB", "negativeComments");
    // SetIterator<reddit::Comment> positiveIter =
    //     pdbClient.getSetIterator<reddit::Comment>("redditDB", "positiveComments");
    // int negativeCount = 0;
    // int positiveCount = 0;
    // for (const auto &nr : negativeIter) {
    //     ++negativeCount;
    // }
    // for (const auto &pr : positiveIter) {
    //     ++positiveCount;
    // }
    // std::cout << "Number of comments with negative set are " << negativeCount << std::endl;
    // std::cout << "Number of comments with positive set are " << positiveCount << std::endl;
    return 0;
}