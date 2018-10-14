
#ifndef SEQUENCEID_H
#define SEQUENCEID_H

#include <pthread.h>

class SequenceID {
private:
    unsigned int currentID;
    pthread_mutex_t sequenceIDMutex;

public:
    SequenceID() {
        currentID = 0;
        pthread_mutex_init(&sequenceIDMutex, nullptr);
    }

    ~SequenceID() {
        pthread_mutex_destroy(&sequenceIDMutex);
    }

    void initialize(unsigned int currentID) {
        this->currentID = currentID;
    }

    unsigned int getNextSequenceID() {
        pthread_mutex_lock(&sequenceIDMutex);
        unsigned int nextID = currentID;
        currentID++;
        pthread_mutex_unlock(&sequenceIDMutex);
        return nextID;
    }
};


#endif /* SEQUENCEID_H */
