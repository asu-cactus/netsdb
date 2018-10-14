#ifndef PDB_EVICT_WORK_CC
#define PDB_EVICT_WORK_CC

#include "PDBEvictWork.h"

PDBEvictWork::PDBEvictWork(PageCache* cache) {
    this->cache = cache;
}

PDBEvictWork::~PDBEvictWork() {}

void PDBEvictWork::execute(PDBBuzzerPtr callerBuzzer) {
    this->cache->evict();
    callerBuzzer->buzz(PDBAlarm::WorkAllDone);
}

#endif
