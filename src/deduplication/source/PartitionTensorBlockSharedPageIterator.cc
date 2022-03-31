#include "FFMatrixBlock.h"
#include "PDBDebug.h"
#include "PartitionTensorBlockSharedPageIterator.h"
#include "FFMatrixBlockIndex.h"

namespace pdb {

/**
 * To create a new PartitionTensorBlockSharedPageIterator instance
 */
PartitionTensorBlockSharedPageIterator::PartitionTensorBlockSharedPageIterator(PageCachePtr cache,
                          PartitionedFilePtr fileOfSharingSet,
                          PartitionedFilePtr fileOfSharedSet,
                          FilePartitionID partitionIdOfSharedSet,
                          SharedFFMatrixBlockSetPtr sharedSet,
			  DatabaseID dbIdOfSharingSet,
			  UserTypeID typeIdOfSharingSet,
			  SetID setIdOfSharingSet) {
    this->cache = cache;
    this->fileOfSharingSet = fileOfSharingSet;
    this->fileOfSharedSet = fileOfSharedSet;
    this->partitionId = partitionIdOfSharedSet;
    this->sharedSet = sharedSet;
    this->dbIdOfSharingSet = dbIdOfSharingSet;
    this->typeIdOfSharingSet = typeIdOfSharingSet;
    this->setIdOfSharingSet = setIdOfSharingSet;
    this->sharedPageMap = this->fileOfSharingSet->getSharedPageMap(partitionId);
    this->it = sharedPageMap->begin();
    this->numPages = sharedPageMap->size();
    this->numIteratedPages = 0;
    std::cout << "PartitionTensorBlockSharedPageIterator: " << numPages << " to scan in partition-" << partitionId << std::endl;

}

/**
 * To return the next page. If there is no more page, return nullptr.
 */
PDBPagePtr PartitionTensorBlockSharedPageIterator::next() {
    PDBPagePtr pageToReturn;
    if (it != this->sharedPageMap->end()) {
        PageID pageId = it->first;
        PageIndex pageIndex = it->second;
        std::cout << this->partitionId << ": PartitionedTensorBlockSharedPageIterator: curTypeId=" << this->fileOfSharedSet->getTypeId()
                     << ",curSetId=" << this->fileOfSharedSet->getSetId()
                     << ",curPageId=" << pageId << ",partitionId="<<pageIndex.partitionId << ",pageSeqId=" << pageIndex.pageSeqInPartition << "\n";
            pageToReturn = cache->getPage(this->fileOfSharedSet,
                                          pageIndex.partitionId,
                                          pageIndex.pageSeqInPartition,
                                          pageId,
                                          false,
                                          nullptr);
	    //MODIFY THE BLOCK's METADATA HERE
	    //Fetch the vector
	    Record<Vector<Handle<FFMatrixBlock>>>*myRec = (Record<Vector<Handle<FFMatrixBlock>>>*)pageToReturn->getBytes();
	    Handle<Vector<Handle<FFMatrixBlock>>> iterateOverMe = myRec->getRootObject();
	    for (int i = 0; i < iterateOverMe->size(); i++) {
	        Handle<FFMatrixBlock> block = (*iterateOverMe)[i];
                Handle<FFMatrixMeta> targetMeta = sharedSet->getTargetMetadata(dbIdOfSharingSet, typeIdOfSharingSet, setIdOfSharingSet, *(block->meta));
                block->meta->blockRowIndex = targetMeta->blockRowIndex;
		block->meta->blockColIndex = targetMeta->blockColIndex;
	    }
            this->numIteratedPages++;
	    it++;
    }
    return pageToReturn;
}

/**
 * If there is more page, return true, otherwise return false.
 */
bool PartitionTensorBlockSharedPageIterator::hasNext() {
    if ((it != this->sharedPageMap->end())&&(this->numIteratedPages < numPages)) {
        return true;
    } else {
        return false;
    }
}

}
