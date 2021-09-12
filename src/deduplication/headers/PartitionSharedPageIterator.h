/*
 * PartitionSharedPageIterator.h
 *
 */

#ifndef SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONSHAREDPAGEITERATOR_H_
#define SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONSHAREDPAGEITERATOR_H_

#include "PageIterator.h"
#include "PDBFile.h"
#include "PageCache.h"
#include "UserSet.h"

class PartitionSharedPageIterator : public PageIteratorInterface {

public:
    /**
     * To create a new PartitionSharedPageIterator instance
     */
    PartitionSharedPageIterator(PageCachePtr cache,
                          PDBFilePtr fileOfSharingSet,
			  PDBFilePtr fileOfSharedSet,
                          FilePartitionID partitionIdOfSharedSet,
			  SharedUserSetPtr sharedSet,
                          UserSet* sharingSet = nullptr);
    /*
     * To support polymorphism.
     */
    ~PartitionSharedPageIterator(){};

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
    PDBFilePtr file;
    FileType type;
    PartitionedFilePtr partitionedFileOfSharingSet;
    PartitionedFilePtr partitionedFileOfSharedSet;
    FilePartitionID partitionId;
    unsigned int numPages = 0;
    unsigned int numIteratedPages = 0;
    SharedUserSetPtr sharedSet;
    UserSet* sharingSet;
};


#endif /* SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONSHAREDPAGEITERATOR_H_ */
