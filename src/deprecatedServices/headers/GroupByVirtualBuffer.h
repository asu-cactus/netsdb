/*
 * File: GroupByVirtualBuffer.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 21:25 PM
 */

#ifndef GROUPBYVIRTUALBUFFER_H
#define GROUPBYVIRTUALBUFFER_H

#include "DataTypes.h"
#include "SmallPage.h"
#include "PDBBackEndServer.h"
#include "FixedSizeSmallPageAllocator.h"
#include <unordered_map>
#include <memory>
using namespace std;


class GroupByVirtualBuffer;
typedef shared_ptr<GroupByVirtualBuffer> GroupByVirtualBufferPtr;

class GroupByVirtualBuffer {

public:
    GroupByVirtualBuffer (int workerId, PDBBackEndServer * server);
    ~GroupByVirtualBuffer();
    bool addObjectByBytes(string key, KeyRangeID rangeId, void * bytes, size_t size);
    SetID getTempSetId(string key);

private:
    unordered_map<string, vector<SmallPagePtr>*> *  groupByMap;
    unordered_map<KeyRangeID, FixedSizeSmallPageAllocatorPtr> * allocators;
    int workerId;
    PDBBackEndServer * server;
    PDBLoggerPtr logger;
};


#endif
