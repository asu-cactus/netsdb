
#ifndef PDB_WORK_C
#define PDB_WORK_C

#include "PDBWork.h"
#include <pthread.h>
#include <iostream>
namespace pdb {

void PDBWork::execute(PDBWorkerQueue* parentIn, PDBBuzzerPtr callerBuzzer) {
    parent = parentIn;
    execute(callerBuzzer);
}

PDBWorkerPtr PDBWork::getWorker() {
    return parent->getWorker();
}

PDBLoggerPtr PDBWork::getLogger() {
    return parent->getLogger();
}

PDBBuzzerPtr PDBWork::getLinkedBuzzer() {
    return {make_shared<PDBBuzzer>([](PDBAlarm myAlarm) {})};
}
}
#endif
