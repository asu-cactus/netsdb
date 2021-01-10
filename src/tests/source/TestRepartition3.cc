#include "UserSet.h"
#include "JoinComp.h"
#include "RedditComment.h"

using namespace pdb;



int main(int argc, char* argv[]) {

NodeID nodeId = 0;
DatabaseID dbId = 2;
UserTypeID typeId = 0;
SetID setId = 3;
std::string metaPartitionPath = argv[1];
pdb::PDBLoggerPtr logger = make_shared<pdb::PDBLogger>("log.out");
PartitionedFilePtr file = std::make_shared<PartitionedFile> (nodeId, dbId, typeId, setId, metaPartitionPath, logger);
file->buildMetaDataFromMetaPartition(nullptr);
file->initializeDataFiles();
int numPages = file->getNumFlushedPages();
size_t pageSize = file->getPageSize();
file->openData();
std::cout << "We have " << numPages << " pages with pageSize=" << pageSize << std::endl;
for (int i = 0; i < numPages; i++) {
   //fetch a page
   char * page = (char*) malloc (pageSize);
   unsigned int pageSeqInPartition = i;
   PageID idToLoad = file->loadPageId(0, i);
   size_t ret = file->loadPage(0, i, page, pageSize);
   std::cout << "have loaded a page with id=" << idToLoad << " with ret=" << ret << std::endl;
   //go through the records in the page
   PDBPagePtr myPage = std::make_shared<PDBPage>(page, 0, 0);
   std::cout << "created an iterator" << std::endl;
   RecordIteratorPtr myIter = std::make_shared<RecordIterator>(myPage);
   if (page != nullptr){
        while (myIter->hasNext() == true) {
             std::cout << "We have found a record" << std::endl;
             Record<Vector<Handle<JoinMap<JoinTuple<reddit::Comment, char [0]>>>>> * myRecord = (Record<Vector<Handle<JoinMap<JoinTuple<reddit::Comment, char [0]>>>>> *)(myIter->next());
             if (myRecord != nullptr) {
                 Handle<Vector<Handle<JoinMap<JoinTuple<reddit::Comment, char [0]>>>>> iterateOverMe = myRecord->getRootObject();
                 int numRecords = iterateOverMe->size();
                 std::cout << "We have " << numRecords << " records" << std::endl;
                 for (int pos = 0; pos < numRecords; pos++) {
                     Handle<JoinMap<JoinTuple<reddit::Comment, char [0]>>> curJoinMap = (*iterateOverMe)[pos];
                     if (curJoinMap != nullptr) {
                         std::cout << "We've got a non-null map with partitionId=" << curJoinMap->getPartitionId() << ", pos=" << pos <<", size=" << curJoinMap->size() << std::endl;
                         curJoinMap->print();
                     } else {
                         std::cout << "We've got a null map" << std::endl;
                     }
                     pos++;
                 }
              }
         }
   } else {
         std::cout << "Got a null page" << std::endl;
   }

}

}
