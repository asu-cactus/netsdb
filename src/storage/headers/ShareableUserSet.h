#ifndef SRC_CPP_MAIN_DATABASE_HEADERS_SHAREABLEUSERSET_H_
#define SRC_CPP_MAIN_DATABASE_HEADERS_SHAREABLEUSERSET_H_

#include "DataTypes.h"
#include "PDBDebug.h"
#include "PDBLogger.h"
#include "PageCache.h"
#include "PageCircularBuffer.h"
#include "PageIterator.h"
#include "PartitionedFile.h"
#include "SequenceID.h"
#include <memory>
#include <set>
#include <vector>

#include "PartitionedShareableFileMetaData.h"
#include "UserSet.h"

using namespace std;

class ShareableUserSet;
typedef shared_ptr<ShareableUserSet> ShareableSetPtr;

class ShareableUserSet : public UserSet {
public:
  /**
   * Create a UserSet instance, need to set file, page cache, and open file
   * later
   */
  ShareableUserSet(pdb::PDBLoggerPtr logger, SharedMemPtr shm, NodeID nodeId,
                   DatabaseID dbId, UserTypeID typeId, SetID setId,
                   string setName, PageCachePtr pageCache,
                   map<SetID, SetPtr>* typeSets,
                   LocalityType localityType = JobData,
                   LocalitySetReplacementPolicy policy = LRU,
                   OperationType operation = Read,
                   DurabilityType durability = TryCache,
                   PersistenceType persistence = Persistent,
                   size_t pageSize = DEFAULT_PAGE_SIZE, size_t desiredSize = 1);

  /**
   * Create a UserSet instance, file needs to be set here
   */
  ShareableUserSet(size_t pageSize, pdb::PDBLoggerPtr logger, SharedMemPtr shm,
                   NodeID nodeId, DatabaseID dbId, UserTypeID typeId,
                   SetID setId, string setName, PartitionedFilePtr file,
                   PageCachePtr pageCache,
                   PartitionedShareableFileMetaDataPtr sharedMetaPtr,
                   map<SetID, SetPtr>* typeSets,
                   LocalityType localityType = JobData,
                   LocalitySetReplacementPolicy policy = LRU,
                   OperationType operation = Read,
                   DurabilityType durability = TryCache,
                   PersistenceType persistence = Persistent,
                   size_t desiredSize = 1);

  PartitionedShareableFileMetaDataPtr getShareableMetaPtr();

  void test() {
    sharedMetaPtr->open_shared_files(typeSets, logger, pageCache);
  }

  SetPtr getSharedSet(SetID setId);


  vector<PageIteratorPtr>* getIterators() override;

private:
  PartitionedShareableFileMetaDataPtr sharedMetaPtr;
  map<SetID, SetPtr>* typeSets;
};

#endif