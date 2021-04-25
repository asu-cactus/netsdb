
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
      numPages(sharedSet->getShareableMetaPtr()->getTotalPageCount()) {
          sharedSet->getShareableMetaPtr()->dump();
      }

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

  assert(set != nullptr);

  PageIndex pidx = set->getFile()->getMetaData()->getPageIndex(pid.pageId);

  std::cout << "SharedPageIterator: curTypeId="
            << sharedSet->getFile()->getTypeId()
            << ",curSetId=" << sharedSet->getFile()->getSetId() << " -> "
            << pid.setId << ",curPageId=" << pid.pageId << "\n";

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
