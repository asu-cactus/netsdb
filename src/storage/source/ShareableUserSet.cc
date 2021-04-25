#ifndef SHAREABLE_USER_SET_CC
#define SHAREABLE_USER_SET_CC

#include "ShareableUserSet.h"
#include "PDBDebug.h"
#include "PartitionPageIterator.h"
#include "SetCachePageIterator.h"
#include "UserSet.h"
#include <cstring>

#include "SharedPageInfo.h"
#include "SharedPageIterator.h"
/**
 * Create a UserSet instance, need to set file, and open file later
 */
ShareableUserSet::ShareableUserSet(
    pdb::PDBLoggerPtr logger, SharedMemPtr shm, NodeID nodeId, DatabaseID dbId,
    UserTypeID typeId, SetID setId, string setName, PageCachePtr pageCache, map<SetID, SetPtr>* typeSets,
    LocalityType localityType, LocalitySetReplacementPolicy policy,
    OperationType operation, DurabilityType durability,
    PersistenceType persistence, size_t pageSize, size_t desiredSize)
    : UserSet(logger, shm, nodeId, dbId, typeId, setId, setName, pageCache,
              localityType, policy, operation, durability, persistence,
              pageSize, desiredSize),
      sharedMetaPtr(nullptr), typeSets(typeSets) {}

/**
 * Create a UserSet instance.
 */
ShareableUserSet::ShareableUserSet(
    size_t pageSize, pdb::PDBLoggerPtr logger, SharedMemPtr shm, NodeID nodeId,
    DatabaseID dbId, UserTypeID typeId, SetID setId, string setName,
    PartitionedFilePtr file, PageCachePtr pageCache,
    PartitionedShareableFileMetaDataPtr sharedMetaPtr, map<SetID, SetPtr>* typeSets,
    LocalityType localityType, LocalitySetReplacementPolicy policy,
    OperationType operation, DurabilityType durability,
    PersistenceType persistence, size_t desiredSize)
    : UserSet(pageSize, logger, shm, nodeId, dbId, typeId, setId, setName, file,
              pageCache, localityType, policy, operation, durability,
              persistence, desiredSize),
      sharedMetaPtr(sharedMetaPtr), typeSets(typeSets) {}

PartitionedShareableFileMetaDataPtr ShareableUserSet::getShareableMetaPtr() {
  return sharedMetaPtr;
}

SetPtr ShareableUserSet::getSharedSet(SetID setId) {
    map<SetID, SetPtr>::iterator setIter;
    if ((setIter = typeSets->find(setId)) != typeSets->end()) {
        return setIter->second;
    } else {
        return nullptr;
    }
}

vector<PageIteratorPtr>* ShareableUserSet::getIterators() {
  vector<PageIteratorPtr>* retVec = UserSet::getIterators();
  PageIteratorPtr iterator = make_shared<SharedPageIterator>(this->pageCache, this);
  retVec->push_back(iterator);

  return retVec;
}

#endif