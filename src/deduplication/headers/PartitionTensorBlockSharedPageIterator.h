/*
 * PartitionTensorBlockSharedPageIterator.h
 *
 */

#ifndef SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONTENSORBLOCKSHAREDPAGEITERATOR_H_
#define SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONTENSORBLOCKSHAREDPAGEITERATOR_H_

#include "PageIterator.h"
#include "PartitionedFile.h"
#include "PageCache.h"
#include "UserSet.h"
#include "SharedTensorBlockSet.h"

class PartitionTensorBlockSharedPageIterator : public PageIteratorInterface {

public:
    /**
     * To create a new PartitionTensorBlockSharedPageIterator instance
     */
    PartitionTensorBlockSharedPageIterator(PageCachePtr cache,
                          PartitionedFilePtr fileOfSharingSet,
			  PartitionedFilePtr fileOfSharedSet,
                          FilePartitionID partitionIdOfSharedSet,
			  SharedTensorBlockSetPtr sharedSet,
                          UserSet* sharingSet = nullptr);
    /*
     * To support polymorphism.
     */
    ~PartitionTensorBlockSharedPageIterator(){};

    /**
     * To return the next page. If there is no more page, return nullptr.
     */
    PDBPagePtr next();

    /**
     * If there is more page, return true, otherwise return false.
     */
    bool hasNext();

private:
    PageCachePtr cache;
    PartitionedFilePtr partitionedFileOfSharingSet;
    PartitionedFilePtr partitionedFileOfSharedSet;
    FilePartitionID partitionId;
    unsigned int numPages = 0;
    unsigned int numIteratedPages = 0;
    SharedTensorBlockSetPtr sharedSet;
    UserSet* sharingSet;
    std::unordered_map<PageID, PageIndex> & sharedPageMap;
    std::unordered_map<PageID, PageIndex>::iterator it;
};


#endif /* SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONTENSORBLOCKSHAREDPAGEITERATOR_H_ */
