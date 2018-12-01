/*
 * File: LargePageWithVariableSizeSmallPages.cc
 * Author: Jia
 *
 * Created on Apr 5, 2016, 21:20 PM
 */

#ifndef LARGEPAGEWITHVARIABLESIZESMALLPAGES_CC
#define LARGEPAGEWITHVARIABLESIZESMALLPAGES_CC

#include "LargePageWithVariableSizeSmallPages.h"

LargePageWithVariableSizeSmallPages::LargePageWithVariableSizeSmallPages(PDBPagePtr page) {
    this->curPage = page;
    this->smallPages = new vector<AggregationSmallPagePtr>();
}

LargePageWithVariableSizeSmallPages::~LargePageWithVariableSizeSmallPages() {
    if(this->smallPages != nullptr) {
        this->clearSmallPages();
    }
    delete this->smallPages;
}

PDBPagePtr LargePageWithVariableSizeSmallPages::getPage() {
    return this->curPage;
}


SmallPagePtr LargePageWithVariableSizeSmallPages::addSmallPage(size_t size) {
    void * bytes = this->curPage->addVariableBytes(size+sizeof(int));
    if(bytes == nullptr) {
        return nullptr;
    }
    SmallPagePtr smallPage = make_shared<SmallPage>(bytes, size, VariableSize);
    return smallPage;
}

AggregationSmallPagePtr LargePageWithVariableSizeSmallPages::addAggregationSmallPage(char * bytes, size_t size) {
    void * dest = this->curPage->addVariableBytes(size);
    if(dest == nullptr) {
        return nullptr;
    }
    AggregationSmallPagePtr smallPage = make_shared<AggregationSmallPage> (bytes, size, dest);
    this->smallPages->push_back(smallPage);
    return smallPage;
}

void LargePageWithVariableSizeSmallPages::clearSmallPages() {
    int i = 0;
    for (i = 0; i<this->smallPages->size(); i++) {
        smallPages->at(i)->setUnpinned();
    }
    smallPages->clear();
    delete this->smallPages;
    this->smallPages = nullptr;
}



void * LargePageWithVariableSizeSmallPages::addBytes(size_t size) {
    return this->curPage->addVariableBytes(size);
}

bool LargePageWithVariableSizeSmallPages::isDirty() {
    return this->curPage->isDirty();
}


#endif
