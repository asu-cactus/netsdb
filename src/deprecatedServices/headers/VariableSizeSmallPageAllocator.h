/*
 * File: VariableSizeSmallPageAllocator.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 22:14 PM
 */

#ifndef VARIABLESIZESMALLPAGEALLOCATOR_H
#define VARIABLESIZESMALLPAGEALLOCATOR_H

#include "PDBBackEndServer.h"
#include "DataTypes.h"
#include "PDBLogger.h"
#include "AggregationSmallPage.h"
#include "LargePageWithVariableSizeSmallPages.h"
#include "BackEndCommunicator.h"
#include "DataProxy.h"
#include "PageHandle.h"
#include "SmallPageAllocator.h"
#include <map>
#include <vector>
#include <memory>
using namespace std;
class VariableSizeSmallPageAllocator;
typedef shared_ptr<VariableSizeSmallPageAllocator> VariableSizeSmallPageAllocatorPtr;

class VariableSizeSmallPageAllocator : public SmallPageAllocator {
public:
    VariableSizeSmallPageAllocator( PDBBackEndServer * server, SetID outputTempSetId, PDBLoggerPtr logger);
    ~VariableSizeSmallPageAllocator();
    AggregationSmallPagePtr getNewAggregationSmallPage(char * bytes, size_t size);
    void * getNewBytes(size_t size);
    SetID getTempSetId() { return this->outputTempSetId; }
    void unpinCurLargePage();
    int getNumPages() { return this->numPages; }
    vector<PageHandlePtr> * getFullLargePages() override; 
    void setUnpinPageIfFull(bool unpinPageIfFull) { this->unpinPageIfFull = unpinPageIfFull; }
    PageHandlePtr getCurPageHandle() override; 
    void incNumKeys() { this->numKeys ++; }
    int getNumKeys() { return this->numKeys; } 
protected:
    LargePageVariablePtr getNewLargePage();

private:
    bool unpinPageIfFull;
    LargePageVariablePtr curLargePage;
    SetID outputTempSetId;
    DataProxyPtr proxy;
    PDBBackEndServer * server;
    PDBLoggerPtr logger;
    int numPages;
    vector<PageHandlePtr> * fullLargePages;
    int numKeys;
};


#endif
