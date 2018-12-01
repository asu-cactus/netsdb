/*
 * File: FixedSizeSmallPageAllocator.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 12:46 PM
 */

#ifndef FIXEDSIZESMALLPAGEALLOCATOR_H
#define FIXEDSIZESMALLPAGEALLOCATOR_H

#include "DataTypes.h"
#include "SmallPage.h"
#include "LargePageWithFixedSizeSmallPages.h"
#include "PDBBackEndServer.h"
#include "DataProxy.h"
#include "PageHandle.h"
#include "SmallPageAllocator.h"
#include "Storage.h"
#include <map>
#include <memory>
#include <string.h>
#include <pthread.h>
using namespace std;
class FixedSizeSmallPageAllocator;
typedef shared_ptr<FixedSizeSmallPageAllocator> FixedSizeSmallPageAllocatorPtr;


class FixedSizeSmallPageAllocator : public SmallPageAllocator {
public:
    FixedSizeSmallPageAllocator(PDBBackEndServer* server, SetID outputTempSetId, PDBLoggerPtr logger);
    FixedSizeSmallPageAllocator(PDBStoragePtr storage, SetID outputTempSetId, PDBLoggerPtr logger);
    ~FixedSizeSmallPageAllocator();
    SmallPagePtr getNewSmallPage();
    SetID getTempSetId() { return this->outputTempSetId; }
    void unpinCurLargePage();
    LargePageFixedPtr reuseCurPage();
    PageHandlePtr getCurPageHandle() override;
    PDBPagePtr getCurPage();
    int getNumPages() { return this->numPages; }
    vector<PageHandlePtr> * getFullLargePages() override;
    vector<PDBPagePtr> * getFullLargePagesLocal();

protected:
    LargePageFixedPtr getNewLargePage();
    LargePageFixedPtr tryGetNewLargePage();
private:
    LargePageFixedPtr curLargePage;
    SetID outputTempSetId;
    DataProxyPtr proxy;
    PDBBackEndServer* server;
    PDBLoggerPtr logger;
    vector<PageHandlePtr> * fullLargePages;
    int numPages;
    pthread_mutex_t allocatorLock;
    bool pageUsedUp;
    bool isLocal;
    PDBStoragePtr storage;
    vector<PDBPagePtr> * fullLargePagesLocal;
    TempSetPtr tempSet;
};






#endif
