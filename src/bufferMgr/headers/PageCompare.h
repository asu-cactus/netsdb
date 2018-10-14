

/****************************************************
** COPYRIGHT 2016, Chris Jermaine, Rice University **
**                                                 **
** The MyDB Database System, COMP 530              **
** Note that this file contains SOLUTION CODE for  **
** A1.  You should not be looking at this file     **
** unless you have completed A1!                   **
****************************************************/


#ifndef PAGE_COMP_H
#define PAGE_COMP_H

#include "TableCompare.h"

// so that pages can be put into a map
class PageCompare {

public:
    bool operator()(const pair<MyDB_TablePtr, size_t>& lhs,
                    const pair<MyDB_TablePtr, size_t>& rhs) const {

        TableCompare temp;
        if (temp(lhs.first, rhs.first))
            return true;

        if (temp(rhs.first, lhs.first))
            return false;

        // in this case, the tables are the same
        return lhs.second < rhs.second;
    }
};

#endif
