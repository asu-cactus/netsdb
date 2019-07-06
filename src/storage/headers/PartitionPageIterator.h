/*
 * PartitionPageIterator.h
 *
 *  Created on: Dec 25, 2015
 *      Author: Jia
 */

#ifndef SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONPAGEITERATOR_H_
#define SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONPAGEITERATOR_H_

#include "PageIterator.h"
#include "PDBFile.h"
#include "PageCache.h"
#include "UserSet.h"

class PartitionPageIterator : public PageIteratorInterface {

public:
    /**
     * To create a new PartitionPageIterator instance
     */
    PartitionPageIterator(PageCachePtr cache,
                          PDBFilePtr file,
                          FilePartitionID partitionId,
                          UserSet* set = nullptr);
    /*
     * To support polymorphism.
     */
    ~PartitionPageIterator(){};

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
    PartitionedFilePtr partitionedFile;
    SequenceFilePtr sequenceFile;
    FilePartitionID partitionId;
    unsigned int numPages = 0;
    unsigned int numIteratedPages = 0;
    UserSet* set;
};


#endif /* SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONPAGEITERATOR_H_ */
