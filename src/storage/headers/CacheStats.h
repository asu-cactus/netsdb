
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
        pthread_mutex_init(&accessLock, nullptr);
    }

    ~CacheStats() {
        pthread_mutex_destroy(&hitLock);
        pthread_mutex_destroy(&missLock);
        pthread_mutex_destroy(&evictedLock);
        pthread_mutex_destroy(&cachedLock);
        pthread_mutex_destroy(&accessLock);
    }

    void incHits(bool shared=false) {
        pthread_mutex_lock(&hitLock);
        numHits ++;
        if (shared) {
            std::cout << "[CACHESTATS] SHARED PAGE CACHE HIT!" << std::endl;
            numSharedHits++;
        }
        pthread_mutex_unlock(&hitLock);
    }

    void addCachedSharedPageAccessTime(double time) {
        pthread_mutex_lock(&accessLock);
        cachedSharedPageAccessTime += time;
        totalCachedSharedPagesAccessed++;
        pthread_mutex_unlock(&accessLock);
    }

    void addSharedPageAccessTime(double time) {
        pthread_mutex_lock(&accessLock);
        sharedPageAccessTime += time;
        totalSharedPagesAccessed++;
        pthread_mutex_unlock(&accessLock);
    }

    void addCachedPageAccessTime(double time) {
        pthread_mutex_lock(&accessLock);
        cachedPageAccessTime += time;
        totalCachedPagesAccessed++;
        pthread_mutex_unlock(&accessLock);
    }

    void addPageAccessTime(double time) {
        pthread_mutex_lock(&accessLock);
        pageAccessTime += time;
        totalPagesAccessed++;
        pthread_mutex_unlock(&accessLock);
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
        std::cout << "*******************" << std::endl;
        std::cout << "total Cached Shared Page Access Time: " << cachedSharedPageAccessTime << std::endl;
        std::cout << "total Cached Shared Pages Accessed: " << totalCachedSharedPagesAccessed << std::endl;
        std::cout << "total Uncached Shared Page Access Time: " << sharedPageAccessTime << std::endl;
        std::cout << "total Uncached Shared Pages Accessed: " << totalSharedPagesAccessed << std::endl;
        std::cout << "total Cached Non-Shared Page Access Time: " << cachedPageAccessTime << std::endl;
        std::cout << "total Cached Non-Shared Pages Accessed: " << totalCachedPagesAccessed << std::endl;
        std::cout << "total Uncached Non-Shared Page Access Time: " << pageAccessTime << std::endl;
        std::cout << "total Uncached Non-Shared Pages Accessed: " << totalPagesAccessed << std::endl;
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

    double cachedSharedPageAccessTime = 0.0;
    long totalCachedSharedPagesAccessed = 0;
    double sharedPageAccessTime = 0.0;
    long totalSharedPagesAccessed = 0;
    double cachedPageAccessTime = 0.0;
    long totalCachedPagesAccessed = 0;
    double pageAccessTime = 0.0;
    long totalPagesAccessed = 0;

    pthread_mutex_t hitLock;
    pthread_mutex_t missLock;
    pthread_mutex_t evictedLock;
    pthread_mutex_t cachedLock;
    pthread_mutex_t accessLock;


};
#endif /* CACHESTATS_H */
