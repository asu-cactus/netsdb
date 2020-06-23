#ifndef PARTITIONED_HASH_SET
#define PARTITIONED_HASH_SET


#include "AbstractHashSet.h"
#include <pthread.h>

namespace pdb {

class PartitionedHashSet;
typedef std::shared_ptr<PartitionedHashSet> PartitionedHashSetPtr;


/*
 * This class encapsulates a partitioned hash set, which is a collection of managed blocks, and each
 * block contains a PDBMap.
 * In the end, this class will be removed and replaced by Pangea hash set
 */

class PartitionedHashSet : public AbstractHashSet {

private:
    // the name of this partitioned hash set
    std::string setName;

    // the set of partition pages
    std::vector<void*> partitionPages;

    // the status of partition pages
    // false: the page of the partition hasn't been returned to application by getPage()
    // true: the page of the partition has been returned to application through invoking getPage()
    std::vector<bool> partitionStatus; 

    // the size of each partition page
    size_t pageSize;

    // whether this partitioned hash set has been cleaned
    bool isCleaned;

    // mutex
    pthread_mutex_t myMutex;

public:
    // constructor
    PartitionedHashSet(std::string myName, size_t pageSize) {
        std::cout << "PartitionedHashSet initialized to have pageSize=" << pageSize << std::endl;
        this->setName = myName;
        this->pageSize = pageSize;
        this->isCleaned = false;
        pthread_mutex_init(&myMutex, nullptr);
    }

    // destructor
    ~PartitionedHashSet() {
        if (isCleaned == false) {
            this->cleanup();
        }
        pthread_mutex_destroy(&myMutex);
    }

    // get type of this hash set
    std::string getHashSetType() override {
        return "PartitionedHashSet";
    }

    // get name of this hash set
    std::string getHashSetName() {
        return setName;
    }

    // get page size
    size_t getPageSize() {
        return pageSize;
    }

    // get page for a particular partition
    void* getPage(unsigned int partitionId, bool anyway=false) {
        void* retPtr = nullptr;
        pthread_mutex_lock(&myMutex);
        if (partitionId < partitionPages.size()) {
            if ((partitionStatus[partitionId] == true)&&(anyway == false)) {
                std::cerr << "Attempting to request for a new partition-"<< partitionId << 
                        " page, which indicates that the hash page size is not big enough" << std::endl;
                return nullptr;
            } else {
                retPtr = partitionPages[partitionId];
                partitionStatus[partitionId] = true;
            }
        }
        pthread_mutex_unlock(&myMutex);
        return retPtr;
    }



    // get number of pages
    size_t getNumPages() {
        size_t retNum;
        pthread_mutex_lock(&myMutex);
        retNum = partitionPages.size();
        pthread_mutex_unlock(&myMutex);
        return retNum;
    }

    // add page
    void* addPage() {
        void* block = (void*)malloc(sizeof(char) * pageSize);
        if (block != nullptr) {
            pthread_mutex_lock(&myMutex);
            partitionPages.push_back(block);
            partitionStatus.push_back(false);
            pthread_mutex_unlock(&myMutex);
        }
        return block;
    }

    // clean up all pages
    void cleanup() override {
        if (isCleaned == false) {
            for (int i = 0; i < partitionPages.size(); i++) {
                free(partitionPages[i]);
            }
            isCleaned = true;
#ifdef PROFILING
            std::cout << "partitioned hash set: " << this->setName << " is removed" << std::endl;
#endif
        }
    }

    // get size
    size_t getSize() override {
        return pageSize * partitionPages.size();
    }
};
}


#endif
