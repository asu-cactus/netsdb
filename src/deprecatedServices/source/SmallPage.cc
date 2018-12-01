/*
 * File: SmallPage.cc
 * Author: Jia
 *
 * Created on Apr 5, 2016, 6:15 AM
 */

#ifndef SMALLPAGE_CC
#define SMALLPAGE_CC

#include "SmallPage.h"
#include <string.h>
#include <iostream>

SmallPage::SmallPage(void * bytes, size_t size, SmallPageType type) {
    this->bytes = bytes;
    this->size = size;
    this->type = type;
    if(this->type == FixedSize) {
        this->curOffsetInSmallPage = sizeof(int);
    } else {
        this->curOffsetInSmallPage = 0;
    }
    this->numObjects = 0;
    remainingSize = this->size - this->curOffsetInSmallPage; 
}

SmallPage::~SmallPage() {
    //cout << "number of objects in this small page:"<< this->numObjects << "\n";

}


bool SmallPage::addObject(PDBObjectPtr object) {
    return this->addObjectByBytes(object->getRaw(), object->getSize());
}

#endif

