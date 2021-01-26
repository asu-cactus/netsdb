#include <PDBClient.h>
#include <RedditComment.h>
#include <RedditPositiveLabelSelection.h>
#include <RedditNegativeLabelSelection.h>


int main(int argc, char* argv[]) {
    // Create required objects
    string errMsg;
    pdb::PDBLoggerPtr clientLogger = pdb::make_shared<pdb::PDBLogger>("clientLog");
    pdb::CatalogClient catalogClient(port, managerIp, clientLogger);
    pdb::PDBClient pdbClient(port, managerIp, clientLogger, false, true);
    // Create/Delete required sets
    pdbClient.removeSet("redditDB", "negativeComments", errMsg);
    pdbClient.removeSet("redditDB", "positiveComments", errMsg);
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
    pdb:Handle<pdb::Computation> writeNegativeSet = pdb::makeObject<
        pdb::WriteUserSet<reddit::Comment>>("redditDB", "negativeComments");
    pdb:Handle<pdb::Computation> writePositiveSet = pdb::makeObject<
        pdb::WriteUserSet<reddit::Comment>>("redditDB", "positiveComments");
    writeNegativeSet->setInput(negativeLabel);
    writePositiveSet->setInput(positiveLabel);
    // Execute write operation for both the set
    pdbClient.executeComputations(errMsg, "RedditSelectionParts",
        writeNegativeSet, writePositiveSet);
    return 0;
}