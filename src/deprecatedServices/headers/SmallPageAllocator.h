/*
 * File: SmallPageAllocator.h
 * Author: Jia
 *
 * Created on Apr 15, 2016, 18:30 PM
 */

#ifndef SMALLPAGEALLOCATOR_H
#define SMALLPAGEALLOCATOR_H

#include "PageHandle.h"
#include <vector>
#include <memory>
using namespace std;
class SmallPageAllocator;
typedef shared_ptr<SmallPageAllocator> SmallPageAllocatorPtr;

class SmallPageAllocator {
public:
    virtual PageHandlePtr getCurPageHandle() = 0;
    virtual vector<PageHandlePtr> * getFullLargePages() = 0;

};



#endif
