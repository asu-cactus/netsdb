#ifndef VARIABLE_SIZE_OBJECT_ITERATOR_CC
#define VARIABLE_SIZE_OBJECT_ITERATOR_CC


#include "VariableSizeObjectIterator.h"


VariableSizeObjectIterator::VariableSizeObjectIterator(PDBPagePtr page) {

    this->page = page;
    this->numObjectsIterated = 0;
    this->numObjectsInCurPage = page->getEmbeddedNumObjects();
    std::cout << "#objects=" << this->numObjectsInCurPage << std::endl;
    this->curPosInPage = (char*)page->getBytes();
}

VariableSizeObjectIterator::~VariableSizeObjectIterator() {}

char* VariableSizeObjectIterator::next(size_t & objectDataSize) {

    if (this->curPosInPage == 0) {
        return nullptr;
    }
    if (this->numObjectsInCurPage <= this->numObjectsIterated) {
        return nullptr;
    }
    // 1st field: record data size
    objectDataSize = *((size_t*)this->curPosInPage);

    // 2nd field: record data
    char* ret = (char *)(curPosInPage + sizeof(size_t));

    // compute offset of next record
    this->curPosInPage = this->curPosInPage + sizeof(size_t) + objectDataSize;

    this->numObjectsIterated++;
    return ret;
}

bool VariableSizeObjectIterator::hasNext() {

    
    if (this->numObjectsInCurPage <= this->numObjectsIterated) {
        this->curPosInPage = 0;
        return false;
    } else {
        return true;
    }
}

#endif
