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

  void addSharedPage(DatabaseID dbId, NodeID nodeId, SetID setId, PageID pageId) {
    SharedPageID pid;
    pid.dbId = dbId;
    pid.nodeId = nodeId;
    pid.setId = setId;
    pid.pageId = pageId;
    sharedMetaPtr->addSharedPage(pid);
  }

  void addSharedPage(SharedPageID *opid) {
    SharedPageID pid;
    pid.dbId = opid->dbId;
    pid.nodeId = opid->nodeId;
    pid.setId = opid->setId;
    pid.pageId = opid->pageId;
    sharedMetaPtr->addSharedPage(pid);
  }

  SetPtr getSharedSet(SetID setId);

  void flushSharedMeta() {
    sharedMetaPtr->openMeta();
    sharedMetaPtr->writeMeta();
  }

  vector<PageIteratorPtr>* getIterators() override;

  void clear() {
    sharedMetaPtr->clear();
    getFile()->clear();
  }

private:
  PartitionedShareableFileMetaDataPtr sharedMetaPtr;
  map<SetID, SetPtr>* typeSets;
};

#endif