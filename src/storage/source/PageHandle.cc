
#ifndef PAGE_HANDLE_CC
#define PAGE_HANDLE_CC

#include "PageHandle.h"
#include <iostream>


PageHandle::PageHandle(DataProxyPtr proxy, PDBPagePtr page) {
    this->proxy = proxy;
    this->page = page;
}

PageHandle::~PageHandle() {}

void PageHandle::pin() {
    if (this->page->isPinned()) {
        return;
    }
    // temp page
    if ((this->page->getDbID() == 0) && (this->page->getTypeID() == 0)) {
        proxy->pinTempPage(page->getSetID(), page->getPageID(), this->page);
    }
    // user page
    else {
        proxy->pinUserPage(page->getNodeID(),
                           page->getDbID(),
                           page->getTypeID(),
                           page->getSetID(),
                           page->getPageID(),
                           this->page);
    }
}

void PageHandle::unpin() {
    if (!this->page->isPinned()) {
        cout << "PageHandle: can not unpin because page is already unpinned.\n";
        return;
    }
    // temp page
    if ((this->page->getDbID() == 0) && (this->page->getTypeID() == 0)) {
        proxy->unpinTempPage(page->getSetID(), page);
    }
    // user page
    else {
        proxy->unpinUserPage(
            page->getNodeID(), page->getDbID(), page->getTypeID(), page->getSetID(), page);
    }
    this->page->setPinned(false);
}


void* PageHandle::getRAM() {
    return this->page->getRawBytes();
}

void* PageHandle::getWritableBytes() {
    return this->page->getBytes();
}

size_t PageHandle::getSize() {
    return this->page->getRawSize();
}

size_t PageHandle::getWritableSize() {
    return this->page->getSize();
}

PageID PageHandle::getPageID() {
    return this->page->getPageID();
}

#endif
