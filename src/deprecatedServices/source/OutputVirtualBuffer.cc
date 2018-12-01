/*
 * File: OutputVirtualBuffer.cc
 * Author: Jia
 *
 * Created on Apr 6, 2016, 10:35 AM
 */

#ifndef OUTPUTVIRTUALBUFFER_CC
#define OUTPUTVIRTUALBUFFER_CC


#include "OutputVirtualBuffer.h"

OutputVirtualBuffer::OutputVirtualBuffer (ReducerID reducerId, int workerId, FixedSizeSmallPageAllocatorPtr allocator) {
    this->reducerId = reducerId;
    this->workerId = workerId;
    this->allocator = allocator;
    this->curSmallPage = nullptr;
}


bool OutputVirtualBuffer::addObject(PDBObjectPtr object) {
    return addObjectByBytes(object->getRaw(), object->getSize());
}

SetID OutputVirtualBuffer::getTempSetId() {
    return this->allocator->getTempSetId();
}


#endif
