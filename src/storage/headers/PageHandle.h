/*
 * File:   PageHandle.h
 * Author: Jia
 *
 * Created on November 25, 2015, 1:22 PM
 */

#ifndef PAGEHANDLE_H
#define PAGEHANDLE_H

#include "PDBPage.h"
#include "DataProxy.h"
#include "PDBObject.h"
#include "PDBLogger.h"
#include <memory>
using namespace std;
class PageHandle;
typedef shared_ptr<PageHandle> PageHandlePtr;

// this class wraps interfaces of manipulating a page in backend.
class PageHandle {

public:
    PageHandle(DataProxyPtr proxy, PDBPagePtr page);
    ~PageHandle();

    // must be called before a call to getRAM; after this call (and until the
    // next call to unPin) all calls to getRAM return the same value
    void pin();

    // after a page is unPinned, the storage manager may move it around
    void unpin();

    // returns a pointer to the bytes making up the page
    void* getRAM();

    // returns a pointer to the bytes after meta data
    void* getWritableBytes();

    // gets the page size, in bytes
    size_t getSize();

    // returns the size of writable bytes in the page
    size_t getWritableSize();

    // gets pageId
    PageID getPageID();

private:
    DataProxyPtr proxy;
    PDBPagePtr page;
};


#endif /* PAGEHANDLE_H */
