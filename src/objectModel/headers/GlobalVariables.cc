
#ifndef GLOBAL_VARS_CC
#define GLOBAL_VARS_CC

#include "Allocator.h"
#include "VTableMap.h"

// there are a number of global variables that make the PDB object model work.  All of these
// global variables are defined in this file.

// set to true if we live in a shared library... this is here mostly because we
// want to be able to close shared libraries when we are done with them, but if
// we try to close a shared library within that shared library, badness results
bool inSharedLibrary = false;

// this is the allocator for pdb :: Objects that is associated with the main thread
Allocator mainAllocator;

// since we want to be able to change the allocator, we use a pointer to it
Allocator* mainAllocatorPtr = &mainAllocator;

// these tell us where the call stack for all of the threads in the PDBWorkerQueue
// is located
void* stackBase = nullptr;
void* stackEnd = nullptr;
// there is one VTableMap, used throughout the process.  This is it
VTableMap globalVTable;

// all accesses to the VTableMap go through this pointer.  This is key because
// if we set this pointer within a shared library, the shared library will use
// the process' VTAbleMap
VTableMap* theVTable = &globalVTable;

// the exception thrown when we run out of data
NotEnoughSpace myException;


#endif
