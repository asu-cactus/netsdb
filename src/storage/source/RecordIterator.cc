#ifndef OBJECT_ITERATOR_CC
#define OBJECT_ITERATOR_CC


#include "RecordIterator.h"


RecordIterator::RecordIterator(PDBPagePtr page) {

    this->page = page;
    this->numObjectsIterated = 0;
    this->numObjectsInCurPage = page->getEmbeddedNumObjects();
    this->curPosInPage = (char*)page->getBytes();
}

RecordIterator::~RecordIterator() {}

Record<Object>* RecordIterator::next() {

    if (this->curPosInPage == 0) {
        return nullptr;
    }

    // 1st field: record data size
    size_t objectDataSize = *((size_t*)this->curPosInPage);
    // 2nd field: record data
    Record<Object>* ret = (Record<Object>*)(curPosInPage + sizeof(size_t));
    #ifdef DEBUG_SHUFFLING
    // write the data to a test file
    std::string fileName =
         "RecordIterator_" + std::to_string(this->numObjectsInCurPage) + "_"+std::to_string(numObjectsIterated);
    FILE* myFile = fopen(fileName.c_str(), "w");
    fwrite(curPosInPage + sizeof(size_t), 1, objectDataSize, myFile);
    fclose(myFile);
    #endif
    // compute offset of next record
    this->curPosInPage = this->curPosInPage + sizeof(size_t) + objectDataSize;

    this->numObjectsIterated++;
    return ret;
}

bool RecordIterator::hasNext() {
    if (this->numObjectsInCurPage == this->numObjectsIterated) {
        this->curPosInPage = 0;
        return false;
    } else {
        return true;
    }
}

#endif
