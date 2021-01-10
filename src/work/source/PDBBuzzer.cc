
#ifndef PDB_BUZZ_C
#define PDB_BUZZ_C

#include "PDBBuzzer.h"
#include <iostream>


PDBBuzzer::PDBBuzzer() {

    pthread_cond_init(&waitingSignal, nullptr);
    pthread_mutex_init(&waitingMutex, nullptr);
    stringFunc = nullptr;
    signalSent = false;
}

PDBBuzzer::PDBBuzzer(std::nullptr_t) {
    pthread_cond_init(&waitingSignal, nullptr);
    pthread_mutex_init(&waitingMutex, nullptr);
    stringFunc = nullptr;
    signalSent = false;
}


void PDBBuzzer::buzz(PDBAlarm withMe) {
    pthread_mutex_lock(&waitingMutex);
    if (noStringFunc != nullptr)
        noStringFunc(withMe);
    pthread_cond_signal(&waitingSignal);
    signalSent = true;
    pthread_mutex_unlock(&waitingMutex);
}

void PDBBuzzer::buzz(PDBAlarm withMe, string message) {
    pthread_mutex_lock(&waitingMutex);
    if (stringFunc != nullptr)
        stringFunc(withMe, message);
    pthread_cond_signal(&waitingSignal);
    signalSent = true;
    pthread_mutex_unlock(&waitingMutex);
}

void PDBBuzzer::buzz(PDBAlarm withMe, int& counter) {
    pthread_mutex_lock(&waitingMutex);
    if (intFunc != nullptr)
        intFunc(withMe, counter);
    pthread_cond_signal(&waitingSignal);
    signalSent = true;
    pthread_mutex_unlock(&waitingMutex);
}

void PDBBuzzer::buzz(PDBAlarm withMe, std::atomic_int& counter) {
    pthread_mutex_lock(&waitingMutex);
    if (atomicFunc != nullptr)
        atomicFunc(withMe, counter);

    pthread_cond_signal(&waitingSignal);
    signalSent = true;
    pthread_mutex_unlock(&waitingMutex);
}

void PDBBuzzer::wait() {

    // wait until there is a buzz
    pthread_mutex_lock(&waitingMutex);
    if (signalSent == true) {
        signalSent = false;
        pthread_mutex_unlock(&waitingMutex);
        return;
    }
    pthread_cond_wait(&waitingSignal, &waitingMutex);
    pthread_mutex_unlock(&waitingMutex);
}

PDBBuzzer::PDBBuzzer(std::function<void(PDBAlarm)> noStringFuncIn) {
    pthread_cond_init(&waitingSignal, nullptr);
    pthread_mutex_init(&waitingMutex, nullptr);
    noStringFunc = noStringFuncIn;
    signalSent = false;
}

PDBBuzzer::PDBBuzzer(std::function<void(PDBAlarm, std::string)> stringFuncIn) {
    pthread_cond_init(&waitingSignal, nullptr);
    pthread_mutex_init(&waitingMutex, nullptr);
    stringFunc = stringFuncIn;
    signalSent = false;
}

PDBBuzzer::PDBBuzzer(std::function<void(PDBAlarm, int&)> intFuncIn) {
    pthread_cond_init(&waitingSignal, nullptr);
    pthread_mutex_init(&waitingMutex, nullptr);
    intFunc = intFuncIn;
    signalSent = false;
}


PDBBuzzer::PDBBuzzer(std::function<void(PDBAlarm, std::atomic_int&)> intFuncIn) {
    pthread_cond_init(&waitingSignal, nullptr);
    pthread_mutex_init(&waitingMutex, nullptr);
    atomicFunc = intFuncIn;
    signalSent = false;
}


PDBBuzzer::~PDBBuzzer() {
    pthread_cond_destroy(&waitingSignal);
    pthread_mutex_destroy(&waitingMutex);
}


#endif
