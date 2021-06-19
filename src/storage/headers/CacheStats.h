
#ifndef CACHESTATS_H
#define CACHESTATS_H

#include <pthread.h>
#include <stdlib.h>
#include <iostream>

class CacheStats {
public:
    CacheStats() {
        pthread_mutex_init(&hitLock, nullptr);
        pthread_mutex_init(&missLock, nullptr);
        pthread_mutex_init(&evictedLock, nullptr);
        pthread_mutex_init(&cachedLock, nullptr);
    }

    ~CacheStats() {
        pthread_mutex_destroy(&hitLock);
        pthread_mutex_destroy(&missLock);
        pthread_mutex_destroy(&evictedLock);
        pthread_mutex_destroy(&cachedLock);
    }

    void incHits(bool shared=false) {
        pthread_mutex_lock(&hitLock);
        numHits ++;
        if (shared) numSharedHits++;
        pthread_mutex_unlock(&hitLock);
    }

    void incMisses(bool shared=false) {
        pthread_mutex_lock(&missLock);
        numMisses ++;
        if (shared) numSharedMisses++;
        pthread_mutex_unlock(&missLock);
    }

    void incEvicted() {
        pthread_mutex_lock(&evictedLock);
        numEvicted ++;
        pthread_mutex_unlock(&evictedLock);
    }

    void incShared() {
        pthread_mutex_lock(&evictedLock);
        numShared ++;
        pthread_mutex_unlock(&evictedLock);
    }

    void incCached() {
        pthread_mutex_lock(&cachedLock);
        numCached ++;
        pthread_mutex_unlock(&cachedLock);
    }

    void print() {
        std::cout << "*****************" << std::endl;
        std::cout << "numHits: " << numHits << std::endl;
        std::cout << "numMisses: " << numMisses << std::endl;
        std::cout << "numSharedHits: " << numSharedHits << std::endl;
        std::cout << "numSharedMisses: " << numSharedMisses << std::endl;
        std::cout << "numEvicted: " << numEvicted << std::endl;
        std::cout << "numCached: " << numCached << std::endl;
        std::cout << "numShared: " << numShared << std::endl;
        std::cout << "*****************" << std::endl;
    }

private:
    int numHits = 0;
    int numMisses = 0;
    int numEvicted = 0;
    int numCached = 0;
    int numSharedHits = 0;
    int numSharedMisses = 0;
    int numShared = 0;
    pthread_mutex_t hitLock;
    pthread_mutex_t missLock;
    pthread_mutex_t evictedLock;
    pthread_mutex_t cachedLock;


};
#endif /* CACHESTATS_H */
