/*
 * File: OutputVirtualBuffer.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 11:17 AM
 */

#ifndef OUTPUTVIRTUALBUFFER_H
#define OUTPUTVIRTUALBUFFER_H

#include <memory>
#include "DataTypes.h"
#include "PDBObject.h"
#include "SmallPage.h"
#include "FixedSizeSmallPageAllocator.h"
#include <map>
using namespace std;
class OutputVirtualBuffer;
typedef shared_ptr<OutputVirtualBuffer> OutputVirtualBufferPtr;


/*
 * OutputVirtualBuffer is used for writing intermediate data.
 * The OutputVirtualBuffer can use fixed size small page allocator to allocate small pages from the large page. 
 */
class OutputVirtualBuffer {

public:
    OutputVirtualBuffer (ReducerID reducerId, int workerId, FixedSizeSmallPageAllocatorPtr allocator);
    inline bool addObjectByBytes(void * bytes, size_t size) {
        if(curSmallPage == nullptr) {
            curSmallPage = allocator->getNewSmallPage();
            if(curSmallPage == nullptr) {
               cout << "OutputVirtualBuffer: 1: can not get another small page...\n";
               exit(-1);
            }
        }
        if(curSmallPage->addObjectByBytes(bytes, size)== false) {
            curSmallPage = allocator->getNewSmallPage();
            if(curSmallPage == nullptr) {
                cout << "OutputVirtualBuffer: 2: can not get another small page...\n";
                cout << "OutputVirtualBuffer: 2: size="<<size<<"\n";
                exit(-1);
            }
            return curSmallPage->addObjectByBytes(bytes, size);
        }
        else {
            return true;
       }

    }
    bool addObject(PDBObjectPtr object);
    SetID getTempSetId();
private:
    ReducerID reducerId;
    SmallPagePtr curSmallPage;
    FixedSizeSmallPageAllocatorPtr allocator;
    int workerId;
};

#endif
