/**
 * VirtualHashMap.h
 *
 * Created on: May 29, 2016
 * Author: Jia
 */

#ifndef VIRTUAL_HASHMAP_H
#define VIRTUAL_HASHMAP_H

#include "HashSet.h"
#include "STLScopedAllocator.h"
#include "PageCache.h"
#include <vector>
#include <math.h>
using namespace std;

template < class Key, //Key class
           class T,   //Value class
           class F   //Spill function for KeyValue pair
           /*,
           class Hash = hash<Key>,
           class Pred = equal_to<Key>,
           class Alloc = allocator< pair<const Key, T>>*/
          >
class VirtualHashMap {
public:
    /**
     * Constructor to initialize hashmap.
     * Param #1: numPartitions, int, number of child partitions (1 for leaf node)
     * Param #2: partitionId, int, index of this partition in parent's childPartitions vector (0 for root node)
     * Param #3: level, int, at which hash bit we should split data for partitions
     * Param #4: size, int, estimated size of hashmap (for performance optimization and reduce fragmentation)
     * Param #5, path, string, path to spill data
     * Param #6, storage, PDBStoragePtr, storage instance that manages PageCache and SetID for FileSets
     * Param #7, hashset, HashSetPtr, hash set that allocates and manage hash partition pages from shared memory
     * Param #8, parent, VirtualHashMap<Key, T, Hash, Pred, Alloc> *, parent node (nullptr for root node)
     * Param #9, page, PDBPagePtr, user can provide existing PDBPage instance for memory allocation
     * Param #10, hashmap, unordered_map<Key, T, Hash, Pred, Alloc> *, user can provide existingunordered_map instance for hashmap
     * Param #11, if Param #10 is not null, we must also specify the allocator used by the hashmap, it will be ignored, if Param #10 is set to be nullptr
     */
    VirtualHashMap (int numPartitions, int partitionId, int level, int size, string path, PDBStoragePtr storage, HashSetPtr hashset, VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/> * parent, PDBPagePtr page, unordered_map<Key, T, hash<Key>, equal_to<Key>, STLScopedAllocator<std::pair<Key, T>>> * hashmap, STLScopedAllocator<std::pair<Key,T>> * allocator) {
        this->numPartitions = numPartitions;
        this->level = level;
        this->path = path;
        this->storage = storage;
        this->cache = storage->getCache();
        this->hashset = hashset;
        this->parent = parent;
        this->leafSpillFile = nullptr;
        if (numPartitions == 1) {
            //A new leaf partition created from scratch or created from an existing partition
            this->partitionId = partitionId;
            this->childPartitions = nullptr;

            if(page != nullptr) {
                this->leafPage = page;
            } else {
                CacheKey key;
                key.dbId = hashset->getDatabaseID();
                key.typeId = hashset->getUserTypeID();
                key.setId = hashset->getSetID();
                key.pageId = hashset->getNextPageID();
                this->leafPage = cache->getNewPage(storage->getNodeId(), key);
            }

            if(hashmap != nullptr) {
                this->leafPartition = hashmap;
                this->allocator = allocator;
            } else {
                void * data = leafPage->getRawBytes();
                this->allocator = new STLScopedAllocator<pair<Key, T>>(data, storage->getConf()->getPageSize());
                hash<Key> hasher = hash<Key>();            
                equal_to<Key> key_equal = equal_to<Key>();
                leafPartition = new unordered_map<Key, T, hash<Key>, equal_to<Key>, STLScopedAllocator<std::pair<Key, T>>> (size, hasher, key_equal, (*allocator));
            }

        }
        else {
            //The root partition
            this->partitionId = 0;
            this->childPartitions = new vector<VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/> *>();
            int i;
            for (i = 0; i < numPartitions; i ++) {
                char childPath[512];
                sprintf(childPath, "%s/%d", this->path, i);
                VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/> * hashPartition = new VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/>(1, i, this->level+log2(numPartitions), string(childPath), storage, this, nullptr, nullptr);
                this->childPartitions->push_back(hashPartition); 
            }
        }
    }


    ~VirtualHashMap() {
        this->clear();
    }

    void clear() {
        if(this->numPartitions == 1) {
            //free leafPartition
            if(this->leafPartition != nullptr) {
                this->leafPartition->clear();
                delete this->leafPartition;
                this->leafPartition = nullptr;
            }
            //unpin page
            if(this->leafPage != nullptr) {
                this->cache->decRefCount(leafPage); 
                this->leafPage = nullptr;
            }
            //remove fileSet
            if(this->leafSpillFile != nullptr) {
                this->leafSpillFile->clear();
                this->leafSpillFile = nullptr;
            }   
            //delete allocator
            if(this->allocator != nullptr) {
                delete this->allocator;
                this->allocator = nullptr;
            }
        } else {
           if(this->childPartitions != nullptr) {
               int i;
               for (i=0; i<this->numPartitions; i++) {
                   this->childPartitions[i]->clear();
               }
               this->childPartitions->clear();
               delete this->childPartitions;
               this->childPartitions = nullptr;
           }
        }
    }

    void insert(Key const & key, T const & value) {
        if (this->numPartitions == 1) {
             try {
                 this->leafPartition->insert(std::make_pair(key, value));
             }
             catch (std::bad_alloc &e) {
                 PDBPagePtr splitLeafPage = this->hashset->addPage();
                 if((splitLeafPage == nullptr)||(this->leafSpillFile!=nullptr)) {
                      this->spill();
                      this->leafPartition->insert(std::make_pair(key, value));
                 } else {
                      this->split(splitLeafPage);
                      this->insert(key, value);
                 }                 
             }

        } else {
            int partitionId = this->getPartitionId(key);
            this->getPartition(partitionId)->insert(key, value);
        }
    }
    //if I am leaf
    void split(PDBPagePtr anotherLeafPage) {
       //create a new hash paritition using anotherLeafPage 
       char strPath[500];
       sprintf(strPath, "%s/%d", this->path, 1);
       VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/> * partition = new VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/> (1, 1, this->level+1, 1024*1024, strPath, this->storage, this->hashset, this, anotherLeafPage, nullptr, nullptr);
       //move corresponding key-value pairs to the new leaf partition;
       this->numPartitions = 2;
       for (auto it = this->leafPartition->begin(); it != this->leafPartition->end();) {
           if(this->getPartitionId(it->first)==1) {
                partition->insert(it->first, it->second);
                it = this->leafPartition->erase(it);
           } else {
                ++it;
           }
       }
       //create another leaf partition using remaining hashmap
       sprintf(strPath, "%s/%d", this->path, 0);
       VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/> * anotherPartition = new VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/> (1, 0, this->level+1, 0, strPath, this->storage, this->hashset, this, this->leafPage, this->leafPartition, this->allocator);
       //upgrade current node from leaf node to an intermediate node;
       this->addPartition(anotherPartition);
       this->addPartition(partition);
       this->leafPage = nullptr;
       this->leafPartition = nullptr;
       this->allocator = nullptr;
    }

    void spill() {
        if(this->numPartitions != 1) {
            return;
        }
        else {
            //create a FileSet if needed
            if(this->leafSpillFile == nullptr) {
                this->storage->addFileSet(this->path, this->fileSetId);
                this->leafSpillFile = this->storage->getFileSet(this->fileSetId);
            }
            //iterate the unordered_map, and write the key value pairs to the FileSet
            for (auto it = this->leafPartition->begin(); it != this->leafPartition->end(); ++it) {
                F(it->first, it->second, this->leafSpillFile);
            }
            //refresh the current map
            delete this->leafPartition;
            delete this->allocator;
            void * data = this->leafPage->getRawBytes();
            this->allocator = new STLScopedAllocator<pair<Key, T>>(data, storage->getConf()->getPageSize());
            hash<Key> hasher = hash<Key>();
            equal_to<Key> key_equal = equal_to<Key>();
            this->leafPartition = new unordered_map<Key, T, hash<Key>, equal_to<Key>, STLScopedAllocator<std::pair<Key, T>>> (size, hasher, key_equal, (*allocator));

        }
    }

protected:
    /*
     * To get partition id from the key.
     */
    int getPartitionId(key const & key) {
       size_t keyhash = hash<string>()(key) >> this->level;
       return keyhash % this->numPartitions;        
    }

    /*
     * To get partition based on partition id.
     */
    VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/>* getPartition(int index) {
        if(this->childPartitions == nullptr) {
            return nullptr;
        }
        else if(index >= this->numPartitions) {
            return nullptr;
        }
        else {
            return this->childPartitions->at(index);
        }
    }

    /*
     * To add a partition as child partition.
     * This function will be used for split.
     */
    void addPartition(VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/> * partition) {
        if(this->childPartitions == nullptr) {
            this->childPartitions = new vector<VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/> *>();
        }
        this->childPartitions->push_back(partition);
    }
 
    
private:
    int numPartitions;
    int level;
    VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/>* parent;
    vector<VirtualHashMap<Key, T, F/*, Hash, Pred, Alloc*/>*> * childPartitions;
    string path;
    PageCachePtr cache; 
    PDBStoragePtr storage;
    HashSetPtr hashset;

    //if I am leaf   
    FileSetPtr leafSpillFile;
    SetID fileSetId;
    int partitionId;
    PDBPagePtr leafPage; //needs to unpin the page on clear
    STLScopedAllocator<pair<Key, T>> * allocator;
    unordered_map<Key, T, hash<Key>, equal_to<Key>, STLScopedAllocator<std::pair<Key, T>>> * leafPartition;
};


