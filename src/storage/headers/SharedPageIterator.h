/*
 * PartitionPageIterator.h
 *
 *  Created on: Dec 25, 2015
 *      Author: Jia
 */

#ifndef SRC_CPP_MAIN_DATABASE_HEADERS_SHAREDPAGEITERATOR_H_
#define SRC_CPP_MAIN_DATABASE_HEADERS_SHAREDPAGEITERATOR_H_

#include "PDBFile.h"
#include "PageCache.h"
#include "PageIterator.h"

#include "ShareableUserSet.h"

class SharedPageIterator : public PageIteratorInterface {

public:
  /**
   * To create a new PartitionPageIterator instance
   */
  SharedPageIterator(PageCachePtr cache, ShareableUserSet * sharedSet);
  /*
   * To support polymorphism.
   */
  ~SharedPageIterator(){};

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
  unsigned int numPages = 0;
  unsigned int numIteratedPages = 0;
  ShareableUserSet * sharedSet;
};

#endif /* SRC_CPP_MAIN_DATABASE_HEADERS_PARTITIONPAGEITERATOR_H_ */
