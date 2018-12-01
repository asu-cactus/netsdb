/*
 * File: LargePageWithVariableSizeSmallPages.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 15:51 PM
 */

#ifndef LARGEPAGEWITHVARIABLESIZESMALLPAGES_H
#define LARGEPAGEWITHVARIABLESIZESMALLPAGES_H
#include "SharedMem.h"
#include "PDBPage.h"
#include "SmallPage.h"
#include "AggregationSmallPage.h"
#include <memory>
#include <vector>
using namespace std;
class LargePageWithVariableSizeSmallPages;
typedef shared_ptr<LargePageWithVariableSizeSmallPages> LargePageVariablePtr;

/*
 * The LargePage class wraps a PDBPage instance consists of multiple small pages with variable size.
 **/
class LargePageWithVariableSizeSmallPages{
public:
   LargePageWithVariableSizeSmallPages(PDBPagePtr page);
   ~LargePageWithVariableSizeSmallPages();
   PDBPagePtr getPage();
   SmallPagePtr addSmallPage(size_t size);
   AggregationSmallPagePtr addAggregationSmallPage(char * bytes, size_t size);
   void * addBytes(size_t size);
   bool isDirty();
   void clearSmallPages();
private:
   PDBPagePtr curPage;
   vector<AggregationSmallPagePtr>* smallPages;
};


#endif
 
