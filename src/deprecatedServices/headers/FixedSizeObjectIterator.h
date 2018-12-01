/*
 * File: FixedSizeObjectIterator.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 19:13PM
 */

#ifndef FIXEDSIZEOBJECTITERATOR_H
#define FIXEDSIZEOBJECTITERATOR_H

#include "LargePageWithFixedSizeSmallPages.h"
#include "PDBPage.h"
#include "PDBObject.h"
#include "SharedMem.h"
#include <memory>
using namespace std;

class FixedSizeObjectIterator;
typedef shared_ptr<FixedSizeObjectIterator> FixedSizeObjectIteratorPtr;

/**
 * This class wraps an iterator to iterate objects in a large page consists of multiple small pages with fixed size.
 **/

class FixedSizeObjectIterator {
public:
  FixedSizeObjectIterator(LargePageFixedPtr largePageFixed);
  FixedSizeObjectIterator(PDBPagePtr page, size_t smallPageSize);
  ~FixedSizeObjectIterator();
  bool hasNext();
  PDBObjectPtr next();
  void * nextRaw(size_t & size);

private:
  PDBPagePtr page;
  int numSmallPagesInLargePage;
  int numObjectsInCurSmallPage;
  int curSmallPageSeqId;
  int numSmallPagesIterated;
  int numObjectsIteratedInCurSmallPage;
  size_t smallPageSize;
  void * curPosition;

};

#endif

