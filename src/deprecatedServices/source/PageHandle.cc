/*
 * File:   PageHandle.cc
 * Author: Jia
 *
 */

#ifndef PAGE_HANDLE_CC
#define PAGE_HANDLE_CC

#include "PageHandle.h"
#include <iostream>



PageHandle::PageHandle (DataProxyPtr proxy, PDBPagePtr page) {
    this->proxy = proxy;
    this->page = page;
}

PageHandle::~PageHandle() {
    
}


// add object to page
bool PageHandle::addObject(PDBObjectPtr object, PDBLoggerPtr logger) {
    if(page == nullptr) {
        return false;
    } else {
        return this->page->addObject(object, logger);	
    }
}

// add byte array to page
bool PageHandle::addBytes(void * buffer, size_t size, PDBLoggerPtr logger) {
    if(page == nullptr) {
        return false;
    } else {
        return this->page->addBytes(buffer, size, logger);
    }
}



void PageHandle::pin() {
    if(this->page->isPinned()) {
        return;
    }
    //temp page
    if((this->page->getDbID()==0) &&(this->page->getTypeID()==0)) {
        proxy->pinTempPage(page->getSetID(), page->getPageID(), this->page);
    }
    //user page
    else {
        proxy->pinUserPage(page->getDbID(),page->getTypeID(),
                page->getSetID(), page->getPageID(), this->page);
    }
}

void PageHandle::unpin() {
    if(!this->page->isPinned()) {
        cout<<"PageHandle: can not unpin because page is already unpinned.\n";
        return;
    }
    //temp page
    if((this->page->getDbID()==0) &&(this->page->getTypeID()==0)) {
        //cout << "PageHandle: page->getPageID()="<<page->getPageID()<<"\n";
        proxy->unpinTempPage(page->getSetID(), page);
    }
    //user page
    else {
        proxy->unpinUserPage(page->getNodeID(),page->getDbID(),page->getTypeID(),
                page->getSetID(), page);
    } 
    this->page->setPinned(false);
    proxy->closeUnpinLoop();
}



void * PageHandle::getRAM() {
     return this->page->getRawBytes();
}

size_t PageHandle::getSize() {
    return this->page->getSize();
}

PageID PageHandle::getPageID() {
    return this->page->getPageID();
}

PDBPagePtr PageHandle::pinAndGetPage() {
    if(this->page->isPinned() == false) {
        this->pin();
    }
    return this->page;
}

#endif
