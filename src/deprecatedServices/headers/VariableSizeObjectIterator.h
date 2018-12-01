/*
 * File: VariableSizeObjectIterator.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 19:13PM
 */

#ifndef VARIABLESIZEOBJECTITERATOR_H
#define VARIABLESIZEOBJECTITERATOR_H

#include "LargePageWithVariableSizeSmallPages.h"
#include "PDBPage.h"
#include <memory>
//#include <iostream>
using namespace std;

class VariableSizeObjectIterator;
typedef shared_ptr<VariableSizeObjectIterator> VariableSizeObjectIteratorPtr;

/**
 * This class wraps an iterator to iterate objects in a large page consists of multiple small pages with variable size.
 **/

//below optimization can work on Ubuntu, but can not work on MacOS, because MacOS class method can not read updated static variable values...

//static int numObjects = 0;
//static int numObjectsIterated = 0;
//static char * curPosition = nullptr;

class VariableSizeObjectIterator {
public:
  VariableSizeObjectIterator(LargePageVariablePtr largePageVariable);

  VariableSizeObjectIterator(PDBPagePtr page) {
    this->page = page;
    if(this->page == nullptr) {
        exit(-1);
    }
    numObjects = this->page->getNumObjects();
    numObjectsIterated = 0;
    curPosition = this->page->getRawBytes() + this->page->getMiniPageSize()*1;

  }

  ~VariableSizeObjectIterator() {
    numObjects = 0;
    numObjectsIterated = 0;
    curPosition = nullptr;
  }


  inline bool hasNext() {
    if(numObjectsIterated < numObjects) {
        return true;
    } else {
        return false;
    }
  }
  PDBObjectPtr next();

  inline char * nextRaw(size_t & size) {

    if(numObjectsIterated >= numObjects) {
        return nullptr;
    } 
    
    //to read the size of current object
    char * cur = curPosition;
    size = (size_t)(*(size_t *) cur);
    char * retBytes = cur + sizeof(size_t);
//    cout << "size=" << size << "\n";

    //to update counters
    numObjectsIterated ++;
    curPosition = retBytes + size;

    //return the object
    return retBytes;
  }

private:
  PDBPagePtr page;
  int numObjects = 0;
  int numObjectsIterated = 0;
  char * curPosition = nullptr;
};

#endif
