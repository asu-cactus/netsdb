
/****************************************************
** COPYRIGHT 2016, Chris Jermaine, Rice University **
**                                                 **
** The MyDB Database System, COMP 530              **
** Note that this file contains SOLUTION CODE for  **
** A1.  You should not be looking at this file     **
** unless you have completed A1!                   **
****************************************************/

#ifndef CHECK_LRU_H
#define CHECK_LRU_H

#include "MyDB_PageHandle.h"

// so that pages can be searched based on LRU access time
class CheckLRU {

public:
    bool operator()(const MyDB_PagePtr lhs, const MyDB_PagePtr rhs) const {
        return lhs->timeTick < rhs->timeTick;
    }
};

#endif
