/*
 * File: SmallPageAllocatorPageIterator.cc
 * Author: Jia
 *
 * Created on Apr 13, 2016, 9:35 AM
 */

#ifndef SMALLPAGEALLOCATORPAGEITERATOR_CC
#define SMALLPAGEALLOCATORPAGEITERATOR_CC

#include "SmallPageAllocatorPageIterator.h"

SmallPageAllocatorPageIterator::SmallPageAllocatorPageIterator(SmallPageAllocatorPtr allocator) {
    this->allocator = allocator;
    this->unpinnedPages = allocator->getFullLargePages();
    this->numPagesIterated = 0;
    this->curPage = allocator->getCurPageHandle()->pinAndGetPage();
    this->numPages = this->unpinnedPages->size();
    if(this->curPage != nullptr) {
        this->numPages ++;
    }
    this->curHandle = allocator->getCurPageHandle();
}

SmallPageAllocatorPageIterator::~SmallPageAllocatorPageIterator() {}

bool SmallPageAllocatorPageIterator::hasNext() {
     if(this->numPagesIterated < this->numPages) {
        return true;
     } else {
        return false;
     }
}

PDBPagePtr SmallPageAllocatorPageIterator::next() {
     PDBPagePtr ret = this->curPage;
     this->numPagesIterated ++;
     if(this->numPagesIterated < this->numPages) {
         this->curPage = this->unpinnedPages->at(this->numPagesIterated-1)->pinAndGetPage();
     } 
     else {
         this->curPage = nullptr;
     }
     return ret;
}


PageHandlePtr SmallPageAllocatorPageIterator::nextHandle() {
     PageHandlePtr ret = this->curHandle;
     this->numPagesIterated ++;
     if(this->numPagesIterated < this->numPages) {
         this->curHandle = this->unpinnedPages->at(this->numPagesIterated -1);
         this->curHandle->pin();
     } else {
         this->curHandle = nullptr;
     }
     return ret;
}

#endif
