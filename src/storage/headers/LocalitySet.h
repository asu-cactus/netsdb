
#ifndef LOCALITY_SET_H
#define LOCALITY_SET_H

#include "PDBPage.h"
#include "DataTypes.h"
#include <list>
#include <vector>
#include <memory>
using namespace std;

class LocalitySet;
typedef shared_ptr<LocalitySet> LocalitySetPtr;
/**
 * This class implements the interfaces for LocalitySet.
 * LocalitySet defines the set locality properties, and is mainly used for PageCache eviction.
 */

class LocalitySet {
public:
    /*
     * Constructor
     */
    LocalitySet(LocalityType localityType,
                LocalitySetReplacementPolicy replacementPolicy,
                OperationType operationType,
                DurabilityType durabilityType,
                PersistenceType persistenceType,
                size_t desiredSize = 1);

    /*
     * Destructor
     */
    ~LocalitySet();

    /*
     * To add a newly cached page to the locality set
     */
    void addCachedPage(PDBPagePtr page);

    /*
     * To update a cached page with new access sequenceId
     */
    void updateCachedPage(PDBPagePtr page);

    /*
     * To remove a cached page
     */
    void removeCachedPage(PDBPagePtr page);

    /*
     * To select a page from the locality set for replacement based on the replacement policy, and
     * also remove the page from the list.
     */
    PDBPagePtr selectPageForReplacement();

    vector<PDBPagePtr>* selectPagesForReplacement();

    void pin(LocalitySetReplacementPolicy policy, OperationType operationType);

    void unpin();

      

    /*
     * Getters/Setters
     */

    LocalityType getLocalityType();

    void setLocalityType(LocalityType type);

    LocalitySetReplacementPolicy getReplacementPolicy();

    void setReplacementPolicy(LocalitySetReplacementPolicy policy);

    OperationType getOperationType();

    void setOperationType(OperationType type);

    DurabilityType getDurabilityType();

    void setDurabilityType(DurabilityType type);

    PersistenceType getPersistenceType();

    void setPersistenceType(PersistenceType type);

    bool isLifetimeEnded();

    void setLifetimeEnd(bool lifetimeEnded);

    size_t getDesiredSize();

    void setDesiredSize (size_t size);

    /**
     * Get number of cached pages.
     */
    int getNumCachedPages() {
        return this->numCachedPages;
    }

    /**
     * Set number of cached pages.
     */
    void setNumCachedPages(int numCachedPages) {
        this->numCachedPages = numCachedPages;
    }

    void lockSetCache () {
        pthread_mutex_lock(&localitySetCacheMutex);
    }

    void unlockSetCache () {
        pthread_mutex_unlock(&localitySetCacheMutex);
    }


    /**
     * Set read/write costs.
     */
    void setWriteCost(double writeCost) {
        this->writeCost = writeCost;
    }

    void setReadCost(double readCost) {
        this->readCost = readCost;
    }

    double getWriteCost() {
        return this->writeCost;
    }

    double getReadCost() {
        return this->readCost;
    }


protected:
    /**
     * Cached pages in the set, ordered by access sequenceId;
     * So pop_front for LRU, pop_back for MRU
     */
    list<PDBPagePtr>* cachedPages;


    /*
     * Types of the data in the set:
     * 1. JobData: job input/output
     * 2. ShuffleData: shuffle data
     * 3. HashPartitionData: hash partition data
     * 4. PartialAggregationData: spilled key-value pairs from a hash partition
     *
     * This property should be set at construction time.
     */
    LocalityType localityType;

    /*
     * Replacement policy in the set:
     * 1. LRU
     * 2. MRU
     * 3. Random
     *
     * This property should be set at construction time, and can be modified at pin time.
     */
    LocalitySetReplacementPolicy replacementPolicy;

    /*
     * On-going operation of the set:
     * 1. Read
     * 2. RepeatedRead
     * 3. Write
     *
     * This property should be set at construction time, and can be modified at pin time.
     */
    OperationType operationType;

    /**
     * Durability type of the set:
     * 1. TryCache: flush to disk only when evicting dirty data
     * 2. CacheThrough: flush disk after each page writing
     *
     * This property should be set at construction time.
     */
    DurabilityType durabilityType;

    /**
     * Whether the set is required for persistence:
     * 1. Transient
     * 2. Persistence
     *
     * This property should be set at construction time.
     */
    PersistenceType persistenceType;

    /**
     * If lifetime ends, the data becomes garbage data which has the lowest priority.
     * This property will be set as false by default at construction time, and will be set as true
     * at unpin time.
     */
    bool lifetimeEnded;


    /**
     * DBMIN attribute: desired size
     */
    size_t desiredSize = 1;


    int numCachedPages = 0;

    pthread_mutex_t localitySetCacheMutex;

    double writeCost = 0.0;
    double readCost = 0.0;

};


#endif
