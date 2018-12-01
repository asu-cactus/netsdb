/*
 * File: VariableSizeObjectIterator.cc
 * Author: Jia
 *
 * Created on Apr 5, 2016, 19:19PM
 */

#ifndef VARIABLESIZEOBJECTITERATOR_CC
#define VARIABLESIZEOBJECTITERATOR_CC

#include "LargePageWithVariableSizeSmallPages.h"
#include "PDBPage.h"
#include "VariableSizeObjectIterator.h"
#include <stdlib.h>

VariableSizeObjectIterator::VariableSizeObjectIterator(LargePageVariablePtr largePageVariable) {
    this->page = largePageVariable->getPage();
    if(this->page == nullptr) {
        exit(-1);
    }
    numObjects = this->page->getNumObjects();
    numObjectsIterated = 0;
    curPosition = this->page->getRawBytes() + this->page->getMiniPageSize()*1; 
}



PDBObjectPtr VariableSizeObjectIterator::next() {
    if(numObjectsIterated >= numObjects) {
        return nullptr;     
    } 

    //to read the size of current object
    size_t size = (size_t)(*(size_t *) curPosition);
    curPosition = (char *) curPosition + sizeof(size_t);

    //to get the pointer to bytes
    void * retBytes = curPosition;
    PDBObjectPtr object = make_shared<PDBObject>(retBytes,0,0,0,size);
    //set shared memory offset to a non-zero value to avoid freeing object automatically.
    object->setShmOffset(1);
    //to update counters
    numObjectsIterated ++;
    curPosition = (char *) curPosition + size;
   
    //return the object
    return object;     
}


#endif
 
