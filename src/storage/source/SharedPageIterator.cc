
#include "SharedPageIterator.h"
#include "PDBDebug.h"
#include "ShareableUserSet.h"
#include <cassert>

/**
 * To create a new PartitionPageIterator instance
 */
SharedPageIterator::SharedPageIterator(PageCachePtr cache,
                                       ShareableUserSet * sharedSet)
    : cache(cache), sharedSet(sharedSet), numIteratedPages(0),
      numPages(sharedSet->getShareableMetaPtr()->getTotalPageCount()) {}

/**
 * To return the next page. If there is no more page, return nullptr.
 */
PDBPagePtr SharedPageIterator::next() {
  PDBPagePtr pageToReturn;
  if (this->numIteratedPages >= this->numPages) {
    return nullptr;
  }
  SharedPageID pid = sharedSet->getShareableMetaPtr()->loadSharedPageId(this->numIteratedPages);
  SetPtr set = sharedSet->getSharedSet(pid.setId);

  /*
  * In case where the set we are borrowing from is removed, set can be null.
  * This assertion was hit when a set s2 was removed then the current set was
  * being evicted for removal too.
  * */
  // assert(set != nullptr);
  if (set == nullptr) {
    std::cout << "SharedPageIterator: WARNING: Set ID " << pid.setId << " is missing. Ignoring this page." << std::endl;
    this->numIteratedPages++;
    return nullptr;
  }

  PageIndex pidx = set->getFile()->getMetaData()->getPageIndex(pid.pageId);

  std::cout << "SharedPageIterator: curTypeId="
            << sharedSet->getFile()->getTypeId()
            << ",curSetId=" << sharedSet->getFile()->getSetId() << " -> "
            << pid.setId << ",curPageId=" << pid.pageId << "\n";

    // Check if page exists in cache first
//     if (set->getDirtyPageSet()->count(pid.pageId) != 0) {
//       cache->evictionLock();
//       FileSearchKey searchKey = set->getDirtyPageSet()->at(pid.pageId);
//       if (searchKey.inCache) {

//             CacheKey key;
//             key.dbId = set->getDbID();
//             key.typeId = set->getTypeID();
//             key.setId = set->getSetID();
//             key.pageId = pid.pageId;

//             std::cout << "SharedPageIterator -> SetCachePageIterator: CACHE HIT!: curPageId=" << key.pageId << "\n";
// #ifdef USE_LOCALITY_SET
//             pageToReturn = cache->getPage1(key, set.get());
// #else
//             pageToReturn = cache->getPage1(key, nullptr);
// #endif
//       } else {
//             std::cout << "SharedPageIterator -> SetCachePageIterator: CACHE MISS!: curPageId=" << pid.pageId << "\n";

// #ifdef USE_LOCALITY_SET
//             pageToReturn = cache->getPage(set->getFile(),
//                                                    searchKey.partitionId,
//                                                    searchKey.pageSeqInPartition,
//                                                    pid.pageId,
//                                                    false,
//                                                    set.get());
// #else
//             pageToReturn = cache->getPage(set->getFile(),
//                                                    searchKey.partitionId,
//                                                    searchKey.pageSeqInPartition,
//                                                    pid.pageId,
//                                                    false,
//                                                    nullptr);
// #endif
//             // remove iter
//             set->lockDirtyPageSet();
//             set->getDirtyPageSet()->erase(pid.pageId);
//             set->unlockDirtyPageSet();
//       }
//       cache->evictionUnlock();
//     } else {
// #ifdef USE_LOCALITY_SET
//             pageToReturn = cache->getPage(set->getFile(), pidx.partitionId, pidx.pageSeqInPartition,
//                        pid.pageId, false, set.get());
// #else
//             pageToReturn = cache->getPage(set->getFile(), pidx.partitionId, pidx.pageSeqInPartition,
//                      pid.pageId, false, nullptr);
// #endif
//     }



  // TODO: should we pass original set to cache or the shared set? 
  pageToReturn =
      cache->getPage(set->getFile(), pidx.partitionId, pidx.pageSeqInPartition,
                     pid.pageId, false, set.get());

  PDB_COUT << "SharedPageIterator: got page" << std::endl;
  this->numIteratedPages++;

  return pageToReturn;
}

/**
 * If there is more page, return true, otherwise return false.
 */
bool SharedPageIterator::hasNext() {
  if (this->numIteratedPages < this->numPages) {
    return true;
  } else {
    return false;
  }
}
