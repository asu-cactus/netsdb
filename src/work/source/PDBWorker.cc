
#ifndef PDB_WORKER_C
#define PDB_WORKER_C

#include "Configuration.h"
#include "Allocator.h"
#include "LockGuard.h"
#include "PDBWorker.h"
#include <iostream>

namespace pdb {

PDBWorker::PDBWorker(PDBWorkerQueue* parentIn) : parent(parentIn) {
    pthread_mutex_init(&workerMutex, nullptr);
    pthread_cond_init(&workToDoSignal, nullptr);
    okToExecute = false;
}

PDBWorkerPtr PDBWorker::getWorker() {
    return parent->getWorker();
}

PDBLoggerPtr PDBWorker::getLogger() {
    return parent->getLogger();
}

void PDBWorker::execute(PDBWorkPtr runMeIn, PDBBuzzerPtr buzzWhenDoneIn) {
    const LockGuard guard{workerMutex};
    runMe = runMeIn;
    buzzWhenDone = buzzWhenDoneIn;
    okToExecute = true;
    pthread_cond_signal(&workToDoSignal);
}

void PDBWorker::soundBuzzer(PDBAlarm withMe) {
    if (buzzWhenDone != nullptr) {
        buzzWhenDone->buzz(withMe);
    }
}

void PDBWorker::enter() {

    // wait for someone to tell us it is OK to do work
    {
        const LockGuard guard{workerMutex};
        while (okToExecute == false) {
            pthread_cond_wait(&workToDoSignal, &workerMutex);
        }
    }
    getAllocator().cleanInactiveBlocks((size_t)(DEFAULT_PAGE_SIZE));
    // then do the work
    runMe->execute(parent, buzzWhenDone);
    okToExecute = false;
}

void PDBWorker::reset() {
    runMe = nullptr;
    buzzWhenDone = nullptr;
}

PDBWorker::~PDBWorker() {
    pthread_mutex_destroy(&workerMutex);
    pthread_cond_destroy(&workToDoSignal);
}
}

#endif
