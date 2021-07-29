
#include "SharedPageIterator.h"
#include "PDBDebug.h"
#include "ShareableUserSet.h"
#include <cassert>
#include <chrono>

/**
 * To create a new PartitionPageIterator instance
 */
SharedPageIterator::SharedPageIterator(PageCachePtr cache,
                                       ShareableUserSet * sharedSet)
    : cache(cache), sharedSet(sharedSet), numIteratedPages(0),
      numPages(sharedSet->getShareableMetaPtr()->getTotalPageCount()) {
          assert(sharedSet->getShareableMetaPtr()->getTotalPageCount() == sharedSet->numShared);
      }

/**
 * To return the next page. If there is no more page, return nullptr.
 */
PDBPagePtr SharedPageIterator::next() {
  auto start = std::chrono::high_resolution_clock::now();

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

  pageToReturn =
      cache->getPage(set->getFile(), pidx.partitionId, pidx.pageSeqInPartition,
                     pid.pageId, false, 
                     set.get());

  PDB_COUT << "SharedPageIterator: got page" << std::endl;
  this->numIteratedPages++;

  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> diff = end-start;

  if (pageToReturn->cached) {
    sharedSet->numSharedHits++;
    cache->addCachedSharedPageAccessTime(diff.count());
  } else {
    sharedSet->numSharedMisses++;
    cache->addSharedPageAccessTime(diff.count());
  }
  
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
