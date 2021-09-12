
#include "PDBDebug.h"
#include "PartitionTensorBlockSharedPageIterator.h"

/**
 * To create a new PartitionTensorBlockSharedPageIterator instance
 */
PartitionTensorBlockSharedPageIterator::PartitionTensorBlockSharedPageIterator(PageCachePtr cache,
                          PDBFilePtr fileOfSharingSet,
                          PDBFilePtr fileOfSharedSet,
                          FilePartitionID partitionIdOfSharedSet,
                          SharedTensorBlockSetPtr sharedSet) {
    this->cache = cache;
    this->partitionedFileOfSharingSet = fileOfSharingSet;
    this->partitionedFileOfSharedSet = fileOfSharedSet;
    this->partitionId = partitionId;
    this->sharedSet = sharedSet;
    sharedPageMap = this->fileOfSharingSet->getSharedPageMap(partitionId);
    it = sharedPageMap.begin();
    this->numPages = sharedPageMap.size();
    this->numIteratedPages = 0;


}

/**
 * To return the next page. If there is no more page, return nullptr.
 */
PDBPagePtr PartitionTensorBlockSharedPageIterator::next() {
    PDBPagePtr pageToReturn;
    if (it != this->sharedPageMap.end()) {
        PageID pageId = it->first;
        PageIndex pageIndex = it->second;
        std::cout << this->partitionId << ": PartitionedTensorBlockSharedPageIterator: curTypeId=" << this->fileOfSharedSet->getTypeId()
                     << ",curSetId=" << this->fileOfSharedSet->getSetId()
                     << ",curPageId=" << pageId << ",partitionId="<<pageIndex.partitionId << ",pageSeqId=" << pageIndex.pageSeqId << "\n";
            pageToReturn = cache->getPage(this->fileOfSharedSet,
                                          this->partitionId,
                                          pageIndex->pageSeqId,
                                          pageIndex->pageId,
                                          false,
                                          nullptr);
            this->numIteratedPages++;
        }
    }
    return pageToReturn;
}

/**
 * If there is more page, return true, otherwise return false.
 */
bool PartitionTensorBlockSharedPageIterator::hasNext() {
    if ((it != this->sharedPageMap.end())&&(this->numIteratedPages < numPages)) {
        return true;
    } else {
        return false;
    }
}
