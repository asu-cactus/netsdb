/*
 * File: FixedSizeObjectIterator.cc
 * Author: Jia
 *
 * Created on Apr 5, 2016, 7:18 AM
 */

#ifndef FIXEDSIZEOBJECTITERATOR_CC
#define FIXEDSIZEOBJECTITERATOR_CC

#include "FixedSizeObjectIterator.h"

FixedSizeObjectIterator::FixedSizeObjectIterator(LargePageFixedPtr largePageFixed){
    this->page = largePageFixed->getPage();
    this->numSmallPagesInLargePage = largePageFixed->getPage()->getNumObjects();
    this->numObjectsInCurSmallPage = 0;
    this->curSmallPageSeqId = -1;
    this->numSmallPagesIterated = 0;
    this->numObjectsIteratedInCurSmallPage = 0;
    this->curPosition = this->page->getRawBytes() + this->page->getMiniPageSize()*1;
    this->smallPageSize = largePageFixed->getSmallPageSize();
}

FixedSizeObjectIterator::FixedSizeObjectIterator(PDBPagePtr page, size_t smallPageSize){
    this->page = page;
    this->numSmallPagesInLargePage = page->getNumObjects();
    this->numObjectsInCurSmallPage = 0;
    this->curSmallPageSeqId = -1;
    this->numSmallPagesIterated = 0;
    this->numObjectsIteratedInCurSmallPage = 0;
    this->curPosition = page->getRawBytes() + page->getMiniPageSize()*1;
    this->smallPageSize = smallPageSize;
}





FixedSizeObjectIterator::~FixedSizeObjectIterator() {}

bool FixedSizeObjectIterator::hasNext() {
    if(this->numSmallPagesIterated < this->numSmallPagesInLargePage) {
        return true;
    } else {
        return false;
    }

}

PDBObjectPtr FixedSizeObjectIterator::next() {
   if(this->numSmallPagesIterated >= this->numSmallPagesInLargePage) {
       return nullptr;
   }

   if((this->curSmallPageSeqId < 0)||(this->numObjectsIteratedInCurSmallPage >= this->numObjectsInCurSmallPage)) {
      //To get the next small page
      if(this->curSmallPageSeqId >= 0) {
          this->numSmallPagesIterated ++;
          this->numObjectsIteratedInCurSmallPage ++;
      }
      this->curPosition = this->page->getRawBytes() + this->page->getMiniPageSize()*1 + this->smallPageSize * this->numSmallPagesIterated;
      //to read the number of objects from curPosition
      this->numObjectsInCurSmallPage = (int)(*(int *) this->curPosition);
      this->curSmallPageSeqId ++;
      this->curPosition = (char*)this->curPosition + sizeof(int);
   }

   //To read the size of current object
   short size =  (short)(*(short *) this->curPosition);
   this->curPosition = (char *)this->curPosition + sizeof(short);
   //To get the pointer to bytes
   void * retBytes = this->curPosition;
   PDBObjectPtr object = make_shared<PDBObject>(retBytes,0,0,0,(size_t)size);
   //set shared memory offset a non-zero value to avoid freeing object automatically.
   object->setShmOffset(1);  
   //To update counters
   this->numObjectsIteratedInCurSmallPage ++;
   this->curPosition = (char *)(this->curPosition) + size;
   //return the object
   return object;

}


void * FixedSizeObjectIterator::nextRaw(size_t & size) {
   if((this->curSmallPageSeqId < 0)||(this->numObjectsIteratedInCurSmallPage >= this->numObjectsInCurSmallPage)) {
      //To get the next small page
      if(this->curSmallPageSeqId >= 0) {
          this->numSmallPagesIterated ++;
      }
      this->curPosition = this->page->getRawBytes() + this->page->getMiniPageSize()*1 + this->smallPageSize * this->numSmallPagesIterated; 
      //to read the number of objects from curPosition
      this->numObjectsInCurSmallPage = (int)(*(int *) this->curPosition);
      this->curSmallPageSeqId ++;
      this->curPosition = (void *)((char *)(this->curPosition) + sizeof(int));
   }

   //To read the size of current object
   size =  (short)(*(short *) this->curPosition);
   this->curPosition = (void *)((char *)(this->curPosition) + sizeof(short)); 
   //To get the pointer to bytes
   void * retBytes = this->curPosition;
   //To update counters
   this->numObjectsIteratedInCurSmallPage ++;
   this->curPosition = (void *)((char *)this->curPosition + size);
   //return the bytes
   return retBytes;
   
} 

#endif
