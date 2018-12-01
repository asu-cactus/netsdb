/*
 * File: SmallPageAllocatorPageIterator.h
 * Author: Jia
 *
 * Created on Apr 13, 2016, 9:12 AM
 */

#ifndef SMALLPAGEALLOCATORPAGEITERATOR_H
#define SMALLPAGEALLOCATORPAGEITERATOR_H

#include "SmallPageAllocator.h"
#include "PageIterator.h"
#include "PDBPage.h"
#include "PageHandle.h"
#include <memory>
using namespace std;

class SmallPageAllocatorPageIterator;
typedef shared_ptr<SmallPageAllocatorPageIterator> SmallPageAllocatorPageIteratorPtr;

/**
 * This class wraps an iterator to iterate all large pages once allocated by a SmallPageAllocator instance.
 **/

class SmallPageAllocatorPageIterator : public PageIteratorInterface {
public:
    SmallPageAllocatorPageIterator(SmallPageAllocatorPtr allocator);
    ~SmallPageAllocatorPageIterator();
    bool hasNext();
    PDBPagePtr next();
    PageHandlePtr nextHandle();
private:
    SmallPageAllocatorPtr allocator;
    int numPagesIterated;
    int numPages;
    vector<PageHandlePtr> * unpinnedPages;
    PDBPagePtr curPage;
    PageHandlePtr curHandle;    
}; 



#endif
