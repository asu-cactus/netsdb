
#ifndef SRC_CPP_MAIN_DATABASE_HEADERS_USERSET_H_
#define SRC_CPP_MAIN_DATABASE_HEADERS_USERSET_H_

#include "PDBDebug.h"
#include "PartitionedFile.h"
#include "PageCache.h"
#include "PDBLogger.h"
#include "DataTypes.h"
#include "PageIterator.h"
#include "PageCircularBuffer.h"
#include "SequenceID.h"
#include <set>
#include <vector>
#include <memory>
#include "LocalitySet.h"
using namespace std;

class UserSet;
typedef shared_ptr<UserSet> SetPtr;

/**
 * This class implements UserSet.
 * A UserType instance include one or more UserSet instances.
 * Each UserSet instance represents a locality group of data,
 * so that all data in a UserSet instance are of the same type
 * and will be accessed together.
 *
 * A UserSet class has following key members:
 * - A point to the global buffer pool (or called PageCache)
 * - A PDBFile instance to manage persisted data
 *
 * Different with BufferSet, a UserSet does not have input buffer to buffer writes. For writing,
 * user can directly pin a new page in buffer pool, so that multi-threaded writing can be more
 * efficient than BufferSet that has single buffer.
 */

class UserSet : public LocalitySet {
public:
    /**
     * Create a UserSet instance, file needs to be set later
     */
    UserSet(pdb::PDBLoggerPtr logger,
            SharedMemPtr shm,
            NodeID nodeId,
            DatabaseID dbId,
            UserTypeID typeId,
            SetID setId,
            string setName,
            PageCachePtr pageCache,
            LocalityType localityType = JobData,
            LocalitySetReplacementPolicy policy = LRU,
            OperationType operation = Read,
            DurabilityType durability = TryCache,
            PersistenceType persistence = Persistent,
            size_t pageSize = DEFAULT_PAGE_SIZE,
            size_t desiredSize = 1,
	    bool isShared = false);

    /**
     * Create a UserSet instance, file is set now
     */
    UserSet(size_t pageSize,
            pdb::PDBLoggerPtr logger,
            SharedMemPtr shm,
            NodeID nodeId,
            DatabaseID dbId,
            UserTypeID typeId,
            SetID setId,
            string setName,
            PartitionedFilePtr file,
            PageCachePtr pageCache,
            LocalityType localityType = JobData,
            LocalitySetReplacementPolicy policy = LRU,
            OperationType operation = Read,
            DurabilityType durability = TryCache,
            PersistenceType persistence = Persistent,
            size_t desiredSize = 1,
	    bool isShared = false);

    /**
     * Destructor.
     */
    ~UserSet();

    /**
     * Add object to a page in the set.
     */
    bool addObject(PDBObjectPtr object, PDBPagePtr page);


    /**
     * Add object to the inputBufferPage in the set.
     */
    bool addObject(PDBObjectPtr object);

    /**
     * pinBufferPage();
     */
    void pinBufferPage();


    /**
     * Unpin buffer page.
     */
    void unpinBufferPage() {
        if (this->inputBufferPage != nullptr) {
            this->inputBufferPage->decRefCount();
        }
    }


    /**
     * Load a page from the set's associate file to cache.
     * If the set's associated file is of SequenceFile type, the parameter partitionId will be
     * ignored.
     * If the page exists in the partition, load it to cache, pin it, and return the page.
     * If the page doesn't exist in the partition, we return nullptr.
     */
    PDBPagePtr getPageFromFile(FilePartitionID partitionId, unsigned int pageSeqInPartition);


    /**
     * Add new page to the page cache for the set.
     */
    PDBPagePtr addPage();

    /**
     * Add new page by existing page raw bytes in shared memory page cache
     */
    PDBPagePtr addPageByRawBytes(size_t sharedMemOffset);


    /**
     * To get next a few bytes with size specified by the parameter.
     * Returns the pointer pointing to the starting position of the bytes.
     */
    inline void* getNewBytes(size_t size, bool evictWhenUnpin = false) {
        if (size == 0) {
            return nullptr;
        }
        pthread_mutex_lock(&this->addBytesMutex);
        if (this->inputBufferPage == nullptr) {

            this->inputBufferPage = this->addPage();
        }
        void* buffer = this->inputBufferPage->addVariableBytes(size);
        if (buffer == nullptr) {
            // current inputBufferPage is full

            // we unpin the inputBufferPage
            this->inputBufferPage->decRefCount();
            if(this->getDurabilityType() == CacheThrough) {
               CacheKey key;
               key.dbId = this->getDbID();
               key.typeId = this->getTypeID();
               key.setId = this->getSetID();
               key.pageId = this->inputBufferPage->getPageID();
               std::cout << "to flush a page in getNewBytes with pageId"<< key.pageId << ", size=" << size << std::endl;
               this->pageCache->flushPageWithoutEviction(key);
            }
            if (evictWhenUnpin == true) {
                this->pageCache->evictPage(this->inputBufferPage);
            }


            // we add a new page as inputBufferPage
            this->inputBufferPage = this->addPage();
            buffer = this->inputBufferPage->addVariableBytes(size);
            pthread_mutex_unlock(&this->addBytesMutex);
            // get new bytes in the new page
            return buffer;
        }
        pthread_mutex_unlock(&this->addBytesMutex);
        return buffer;
    }


    /**
     * Get a set of iterators for scanning the data in the set.
     * The set of iterators will include:
     * -- 1 iterator to scan data in input buffer;
     * -- K iterators to scan data in file partitions, assuming there are K partitions.
     * IMPORTANT: user needs to delete the returned vector!!!
     */
    virtual vector<PageIteratorPtr>* getIterators();


    /**
     * Get a set of iterators for scanning the data in the set, including the pages that are shared by the set but not stored in the set.
     * The set of iterators will include:
     * -- 1 iterator to scan data in input buffer;
     * -- K iterators to scan data in file partitions, assuming there are K partitions.
     * -- K iterators to scan shared pages in file partitions, assuming there are K partitions
     * IMPORTANT: user needs to delete the returned vector!!!
     */
    virtual vector<PageIteratorPtr>* getIteratorsExtended(SetPtr sharedSetPtr);



    /**
     * Get page from set.
     * Step 1. check whether the page is already in cache using cache key, if so return it.
     * Step 2. check whether the page is flushed to disk file, if so, load it to cache, and return
     * it.
     */
    PDBPagePtr getPage(FilePartitionID partitionId, unsigned int pageSeqInPartition, PageID pageId);

    /**
     * Get number of pages.
     */
    int getNumPages();



    /**
     * Return DatabaseID of the set instance.
     */
    DatabaseID getDbID() {
        return this->dbId;
    }


    /**
     * Return UserTypeID of the set instance
     */
    UserTypeID getTypeID() {
        return this->typeId;
    }


    /**
     * Return SetID of the set instance.
     */
    SetID getSetID() {
        return this->setId;
    }

    /**
     * Return the name of the set instance.
     */
    string getSetName() {
        return this->setName;
    }

    /**
     * Returns the number of external pages the set shares
     */
    int getNumSharedPages() {
        return this->file->getNumSharedPages();
    }

    /**
     * Returns file instance;
     */
    PartitionedFilePtr getFile() {
        return this->file;
    }

    /**
     * Set file instance;
     */
    void setFile(PartitionedFilePtr file) {
        this->file = file;
    }

    /**
     * Returns last flushed pageId;
     */
    PageID getLastFlushedPageId() {
        return this->lastFlushedPageId;
    }


    /**
     * Set last flushed pageId;
     */
    void setLastFlushedPageId(PageID pageId) {
        this->lastFlushedPageId = pageId;
    }

    /**
     *
     * Add dirty page to the Set's hash map for dirty pages
     */
    void addPageToDirtyPageSet(PageID pageId) {
        lockDirtyPageSet();
        auto search = dirtyPagesInPageCache->find(pageId);
        if (search != dirtyPagesInPageCache->end()) {
            cout << "UserSet: Error in addPageToDirtyPageSet, key exists.\n";
        } else {
            FileSearchKey key;
            key.inCache = true;
            key.partitionId = (unsigned int)(-1);
            key.pageSeqInPartition = (unsigned int)(-1);
            dirtyPagesInPageCache->insert({pageId, key});
        }
        unlockDirtyPageSet();
    }

    /**
     * Not really remove dirty page from the Set's hash map for dirty pages;
     * Just label it as flushed, and set other fields of FileSearchKey.
     * It is not thread-safe. It is only used in PDBFlushConsumerWork class, where it is guarded by
     * lock.
     */
    void removePageFromDirtyPageSet(PageID pageId,
                                    FilePartitionID partitionId,
                                    unsigned int pageSeqInPartition) {
        if (isPinned == false) {
            PDB_COUT << "the set is not pinned" << std::endl;
            dirtyPagesInPageCache->erase(pageId);
            return;
        }
        PDB_COUT << "the set is pinned" << std::endl;
        auto search = dirtyPagesInPageCache->find(pageId);
        if (search != dirtyPagesInPageCache->end()) {
            search->second.inCache = false;
            search->second.partitionId = partitionId;
            search->second.pageSeqInPartition = pageSeqInPartition;
        }
        return;
    }

    /**
     * Get the dirty page map
     */
    unordered_map<PageID, FileSearchKey>* getDirtyPageSet() {
        return dirtyPagesInPageCache;
    }

    void lockDirtyPageSet() {
        pthread_mutex_lock(&this->dirtyPageSetMutex);
    }

    void unlockDirtyPageSet() {
        pthread_mutex_unlock(&this->dirtyPageSetMutex);
    }


    /**
     * To dump set data to a buffer.
     * NOTE: User MUST guarantee that the size of buffer is large enough for dumping all data in the
     * set.
     */
    void dump(char* buffer);

    void evictPages();

    bool getPinned() {
        return this->isPinned;
    }

    /*
     * TODO: change the name because current name is easy to be confused with pin()/unpin() defined
     * in LocalitySet class (which is the base class).
     */
    void setPinned(bool isPinned) {
        lockDirtyPageSet();
        this->isPinned = isPinned;
        unlockDirtyPageSet();
    }

    pdb::PDBLoggerPtr getLogger() {
        return this->logger;
    }

    void cleanDirtyPageSet();

    // MUST be used after set->setPinned(true) is invoked
    void flushDirtyPages();

    size_t getPageSize() {
        return this->pageSize;
    }

    void setPageSize(size_t pageSize) {
        this->pageSize = pageSize;
    }

    void print() {
        std::cout << "################################" << std::endl;
        if (isShared){
	   std::cout << "SharedSet" << std::endl;
	} else {
	   std::cout << "PrivateSet" << std::endl;
	}
        std::cout << "readCost:" << readCost << std::endl;
	std::cout << "writeCost:" << writeCost << std::endl;
	std::cout << "setName:" << setName << std::endl;
        std::cout << "pageSize:" << pageSize << std::endl;
        switch (localityType) {
            case JobData:
                std::cout << "localityType: JobData" << std::endl;
                break;
            case ShuffleData:
                std::cout << "localityType: ShuffleData" << std::endl;
                break;
            case HashPartitionData:
                std::cout << "localityType: HashPartitionData" << std::endl;
                break;
            case PartialAggregationData:
                std::cout << "localityType: PartialAggregationData" << std::endl;
                break;
            default:
                std::cout << "localityType: " << localityType << std::endl;
                break;           
        }
        switch (replacementPolicy) {
            case LRU: 
                std::cout << "replacementPolicy: LRU" << std::endl;
                break;
            case MRU:
                std::cout << "replacementPolicy: MRU" << std::endl;
                break;
            case Random:
                std::cout << "replacementPolicy: Random" << std::endl;
                break;
            default:
                std::cout << "replacementPolicy: " << replacementPolicy << std::endl;
                break;
        }
        switch (operationType) {
            case Read:     
                std::cout << "operationType: Read" << std::endl;
                break;
            case RepeatedRead:
                std::cout << "operationType: RepeatedRead" << std::endl;
                break;
            case Write:
                std::cout << "operationType: Write" << std::endl;
                break;
            default:
                std::cout << "operationType: " << operationType << std::endl;
                break;
        }
        switch (durabilityType) {
            case TryCache:     
                std::cout << "durabilityType: TryCache" << std::endl;
                break;
            case CacheThrough:
                std::cout << "durabilityType: CacheThrough" << std::endl;
                break;
            default:
                std::cout << "durabilityType: " << durabilityType << std::endl;
                break;
        }
        switch (persistenceType) {
            case Transient:
                std::cout << "persistenceType: Transient" << std::endl;
                break;
            case Persistent:
                std::cout << "persistenceType: Persistent" << std::endl;
                break;
            default:
                std::cout << "persistenceType: " << persistenceType << std::endl;
                break;
        }
        if (lifetimeEnded) {
            std::cout << "lifetimeEnded: true" << std::endl;
        } else {
            std::cout << "lifetimeEnded: false" << std::endl;
        }
        std::cout << "numCachedPages: " << numCachedPages << std::endl;
        std::cout << "################################" << std::endl;

    }


protected:
    PartitionedFilePtr file;
    PageCachePtr pageCache;
    SharedMemPtr shm;
    pdb::PDBLoggerPtr logger;
    NodeID nodeId;
    DatabaseID dbId;
    UserTypeID typeId;
    SetID setId;
    string setName;
    PageID lastFlushedPageId;
    PageID latestPageId;
    PDBPagePtr inputBufferPage;
    SequenceID seqId;
    unordered_map<PageID, FileSearchKey>* dirtyPagesInPageCache;
    pthread_mutex_t dirtyPageSetMutex;
    bool isPinned;
    int numPages = 0;
    pthread_mutex_t addBytesMutex;
    size_t pageSize;
};


#endif /* SRC_CPP_MAIN_DATABASE_HEADERS_PDBSET_H_ */
