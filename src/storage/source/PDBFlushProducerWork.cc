#ifndef PDB_FLUSH_WORK_CC
#define PDB_FLUSH_WORK_CC

#include "PDBFlushProducerWork.h"

PDBFlushProducerWork::PDBFlushProducerWork(pdb::PangeaStorageServer* server) {
    this->server = server;
}

void PDBFlushProducerWork::execute(PDBBuzzerPtr callerBuzzer) {
    callerBuzzer->buzz(PDBAlarm::WorkAllDone);
}

#endif
