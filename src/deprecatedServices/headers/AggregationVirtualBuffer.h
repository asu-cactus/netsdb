/*
 * File: AggregationVirtualBuffer.h
 * Author: Jia
 *
 * Created on Apr 4, 2016, 21:25 PM
 */

#ifndef AGGREGATIONVIRTUALBUFFER_H
#define AGGREGATIONVIRTUALBUFFER_H

#include "AggregationSmallPage.h"
#include "DataTypes.h"
#include "PDBBackEndServer.h"
#include "VariableSizeSmallPageAllocator.h"
#include "PDBLogger.h"
#include <unordered_map>
#include <memory>
using namespace std;


class AggregationVirtualBuffer;
typedef shared_ptr<AggregationVirtualBuffer> AggregationVirtualBufferPtr;


class AggregationVirtualBuffer {

public:
    AggregationVirtualBuffer (int workerId, PDBBackEndServer * server, int numRangePartitions, int iterationId, SetKey outputSetKey, PDBLoggerPtr logger);
    ~AggregationVirtualBuffer();
    bool addOrUpdateObjectByBytes(void * bytes, size_t size, int count);
    SetID getTempSetId(KeyRangeID rangeId);
    string getKey(void * bytes, size_t size);
    KeyRangeID getKeyRangeID(string key);
    SetKey getOutputSetKey();
    void aggregate();
private:
    unordered_map<string, AggregationSmallPagePtr> *  aggregationMap;
    unordered_map<KeyRangeID, VariableSizeSmallPageAllocatorPtr> * allocators;
    int workerId;
    PDBBackEndServer * server;
    PDBLoggerPtr logger;
    int numRangePartitions;
    VariableSizeSmallPageAllocatorPtr aggregationAllocator;
    int iterationId;
    SetKey setKey;
};


#endif
