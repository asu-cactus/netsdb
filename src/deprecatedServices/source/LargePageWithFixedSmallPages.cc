/*
 * File: LargePageWithFixedSizeSmallPages.cc
 * Author: Jia
 *
 * Created on Apr 5, 2016, 12:46 PM
 */

#ifndef LARGEPAGEWITHFIXEDSIZESMALLPAGES_CC
#define LARGEPAGEWITHFIXEDSIZESMALLPAGES_CC

#include "LargePageWithFixedSizeSmallPages.h"

LargePageWithFixedSizeSmallPages::LargePageWithFixedSizeSmallPages(PDBPagePtr page, size_t smallPageSize) {
    this->curPage = page;
    this->smallPageSize = smallPageSize;
}

LargePageWithFixedSizeSmallPages::~LargePageWithFixedSizeSmallPages() {}

PDBPagePtr LargePageWithFixedSizeSmallPages::getPage() {
    return this->curPage;
}

SmallPagePtr LargePageWithFixedSizeSmallPages::addSmallPage() {
    if(curPage == nullptr) {
        return nullptr;
    }
    void * bytes = nullptr;
    //cout << "LargePageWithFixedSizeSmallPages:smallPageSize="<<this->smallPageSize<<"\n";
    bytes = this->curPage->addFixedBytes(this->smallPageSize);
    size_t size;
    SmallPagePtr smallPage;
    if(bytes == nullptr) {
        bytes = this->curPage->getRemainingBytes(size);
        //cout << "size="<<size<<"\n";
        if(bytes == nullptr) {
           //cout << "LargePageWithFixedSizeSmallPages:the page has no bytes\n";
           return nullptr;
        }
        smallPage= make_shared<SmallPage>(bytes, size, FixedSize);
    } else {
        smallPage = make_shared<SmallPage>(bytes, this->smallPageSize, FixedSize);
    }
    return smallPage;
}

bool LargePageWithFixedSizeSmallPages::isDirty() {
    return this->curPage->isDirty();
}

size_t LargePageWithFixedSizeSmallPages::getSmallPageSize() {
    return this->smallPageSize;
}
#endif
