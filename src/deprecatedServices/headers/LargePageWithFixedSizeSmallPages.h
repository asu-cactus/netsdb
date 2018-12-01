/*
 * File: LargePageWithFixedSizeSmallPages.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 15:51 PM
 */

#ifndef LARGEPAGEWITHFIXEDSIZESMALLPAGES_H
#define LARGEPAGEWITHFIXEDSIZESMALLPAGES_H

#include "PDBPage.h"
#include "SmallPage.h"
#include <memory>
using namespace std;
class LargePageWithFixedSizeSmallPages;
typedef shared_ptr<LargePageWithFixedSizeSmallPages> LargePageFixedPtr;

/*
 * The LargePage class wraps a PDBPage instance consists of multiple small pages with fixed size.
 **/
class LargePageWithFixedSizeSmallPages{
public:
   LargePageWithFixedSizeSmallPages(PDBPagePtr page, size_t smallPageSize);
   ~LargePageWithFixedSizeSmallPages();
   PDBPagePtr getPage();
   SmallPagePtr addSmallPage();
   bool isDirty();
   size_t getSmallPageSize();
    
private:
   PDBPagePtr curPage;
   size_t smallPageSize;
};


#endif
 
