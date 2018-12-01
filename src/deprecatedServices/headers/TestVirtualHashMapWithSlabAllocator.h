/**
 * TestVirtualHashMapWithSlabAllocator.h
 *
 * Created on: June 1st, 2016
 * Author: Jia
 */

#ifndef TEST_VIRTUAL_HASHMAP_WITH_SLAB_ALLOCATOR_H
#define TEST_VIRTUAL_HASHMAP_WITH_SLAB_ALLOCATOR_H

#include "HashSet.h"
#include "TempSet.h"
#include "STLSlabAllocator.h"
#include "PageCache.h"
#include "Storage.h"
#include <string>
#include <vector>
#include <math.h>
using namespace std;

class TestVirtualHashMapWithSlabAllocator {
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
     * Param #8, tempset, TempSetPtr, temp set that writes spilled partial aggregation results to disk
     * Param #9, parent, VirtualHashMap<Key, T, Hash, Pred, Alloc> *, parent node (nullptr for root node)
     * Param #10, page, PDBPagePtr, user can provide existing PDBPage instance for memory allocation
     * Param #11, hashmap, unordered_map<Key, T, Hash, Pred, Alloc> *, user can provide existingunordered_map instance for hashmap
     */
    TestVirtualHashMapWithSlabAllocator (int numPartitions, int partitionId, int level, int size, string path, PDBStoragePtr storage, HashSetPtr hashset, TempSetPtr tempset, TestVirtualHashMapWithSlabAllocator* parent, PDBPagePtr page, unordered_map<string, int, hash<string>, equal_to<string>, STLSlabAllocator<std::pair<string, int>>> * hashmap) {
        //cout << "this->partitionId"<<partitionId<<"\n";
        this->numPartitions = numPartitions;
        this->level = level;
        this->size = size;
        this->path = path;
        this->storage = storage;
        this->cache = storage->getCache();
        this->hashset = hashset;
        this->parent = parent;
        this->leafSpillFile = tempset;
        if (numPartitions == 1) {
            //A new leaf partition created from scratch or created from an existing partition
            this->partitionId = partitionId;
            this->childPartitions = nullptr;

            if(page != nullptr) {
                this->leafPage = page;
            } else {
                this->leafPage = this->hashset->addPage();
            }

            if(hashmap != nullptr) {
                this->leafPartition = hashmap;
            } else {
                void * data = leafPage->getRawBytes();
                STLSlabAllocator<pair<string, int>> testAllocator(data, storage->getConf()->getPageSize());
                hash<string> hasher = hash<string>();            
                equal_to<string> key_equal = equal_to<string>();
                leafPartition = new unordered_map<string, int, hash<string>, equal_to<string>, STLSlabAllocator<std::pair<string, int>>> (size, hasher, key_equal, testAllocator);
            }

        }
        else {
            //The root partition
            this->partitionId = 0;
            this->childPartitions = new vector<TestVirtualHashMapWithSlabAllocator *>();
            int i;
            for (i = 0; i < numPartitions; i ++) {
                char childPath[512];
                sprintf(childPath, "%s/%d", this->path.c_str(), i);
                TestVirtualHashMapWithSlabAllocator * hashPartition = new TestVirtualHashMapWithSlabAllocator(1, i, this->level+log2(numPartitions), size, string(childPath), storage, hashset, tempset, this, nullptr, nullptr);
                this->childPartitions->push_back(hashPartition); 
            }
        }
    }


    ~TestVirtualHashMapWithSlabAllocator() {
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
                this->leafPage->decRefCount(); 
                this->leafPage = nullptr;
            }
            //remove fileSet
            /*
            if(this->leafSpillFile != nullptr) {
                this->leafSpillFile->clear();
                this->leafSpillFile = nullptr;
            } 
            */  
        } else {
           if(this->childPartitions != nullptr) {
               int i;
               for (i=0; i<this->numPartitions; i++) {
                   this->childPartitions->at(i)->clear();
                   delete this->childPartitions->at(i);
               }
               this->childPartitions->clear();
               delete this->childPartitions;
               this->childPartitions = nullptr;
           }
        }
    }

    void insert(string key, int value) {
        if (this->numPartitions == 1) {
             try {
                 this->leafPartition->insert(std::pair<string, int>(key, value));
                 //this->leafPartition->[key]=value;
             }
             catch (std::bad_alloc &e) {
                 //cout << "get a new page for spliting current partition with Id="<<this->partitionId<<"\n";
                 PDBPagePtr splitLeafPage = this->hashset->addPage();
                 if((splitLeafPage == nullptr)||(this->spilled == true)) {
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

    int collect(SetPtr outputSet) {
        int numAggregatedPartitions = 0;
        if(this->numPartitions == 1) {
           if(this->spilled == false) {
               //iterate the unordered_map, and write the key value pairs to the FileSet
               for (auto it = this->leafPartition->begin(); it != this->leafPartition->end(); ++it) {
                   size_t length = ((string)(it->first)).length();
                   size_t size = sizeof(int) + length + sizeof(int);
                   void * start = outputSet->getNewBytes(size, false);
                   if(start == nullptr) {
                       cout << "Error: Out of Memory when spill partial aggregation results!\n";
                       exit(-1);
                   }
                   char * cur = (char *) start;
                   * ((int *)cur) = (int)(length);
                   cur += sizeof(int);
                   memcpy(cur, ((string)(it->first)).c_str(), length);
                   cur += length;
                   * ((int *)cur) = it->second;
                   cur += sizeof(int);
               } 
               numAggregatedPartitions ++;
            }
            else {
               this->spill();
            }
            this->hashset->unpinPage(this->leafPage->getPageID());
            if(this->parent != nullptr) {
               this->parent->removeChildPartition(this->partitionId);
            }
            return numAggregatedPartitions;
        } 
        else {
            int i;
            for (i=0; i<numPartitions; i++) {
                numAggregatedPartitions += this->childPartitions->at(i)->collect(outputSet);
            }
            if(this->numPartitions == 0) {
                if(this->parent != nullptr) {
                    this->parent->removeChildPartition(this->partitionId);
                }
            }
            return numAggregatedPartitions;
        }

    }


    int collectAll(SetPtr outputSet) {
        int numAggregatedPartitions = 0;
        if(this->numPartitions == 1) {
            for (auto it = this->leafPartition->begin(); it != this->leafPartition->end(); ++it) {
                size_t length = ((string)(it->first)).length();
                size_t size = sizeof(int) + length + sizeof(int);
                void * start = outputSet->getNewBytes(size, false);
                if(start == nullptr) {
                    cout << "Error: Out of Memory when spill partial aggregation results!\n";
                    exit(-1);
                }
                char * cur = (char *) start;
                * ((int *)cur) = (int)(length);
                cur += sizeof(int);
                memcpy(cur, ((string)(it->first)).c_str(), length);
                cur += length;
                * ((int *)cur) = it->second;
                cur += sizeof(int);
            }
            if(this->spilled == false) {
                numAggregatedPartitions ++;
            } 
            this->hashset->unpinPage(this->leafPage->getPageID());
            if(this->parent != nullptr) {
               this->parent->removeChildPartition(this->partitionId);
            }
            return numAggregatedPartitions;
        }
        else {
            int i;
            for (i=0; i<numPartitions; i++) {
                numAggregatedPartitions += this->childPartitions->at(i)->collectAll(outputSet);
            }
            if(this->numPartitions == 0) {
                if(this->parent != nullptr) {
                    this->parent->removeChildPartition(this->partitionId);
                }
            }
            return numAggregatedPartitions;
        }

    }

    int eraseAll() {
        int numAggregatedPartitions = 0;
        if(this->numPartitions == 1) {
            for (auto it = this->leafPartition->begin(); it != this->leafPartition->end();) {
                this->leafPartition->erase(it);
            }
            if(this->spilled == false) {
                numAggregatedPartitions ++;
            }
            this->hashset->unpinPage(this->leafPage->getPageID());
            if(this->parent != nullptr) {
               this->parent->removeChildPartition(this->partitionId);
            }
            return numAggregatedPartitions;
        }
        else {
            int i;
            for (i=0; i<numPartitions; i++) {
                numAggregatedPartitions += this->childPartitions->at(i)->eraseAll();
            }
            if(this->numPartitions == 0) {
                if(this->parent != nullptr) {
                    this->parent->removeChildPartition(this->partitionId);
                }
            }
            return numAggregatedPartitions;
        }

    }


    int countAll() {
        int count = 0;
        if(this->numPartitions == 1) {
            for (auto it = this->leafPartition->begin(); it != this->leafPartition->end();) {
                count++;
            }
            this->hashset->unpinPage(this->leafPage->getPageID());
            if(this->parent != nullptr) {
               this->parent->removeChildPartition(this->partitionId);
            }
            return count;
        }
        else {
            int i;
            for (i=0; i<numPartitions; i++) {
                count += this->childPartitions->at(i)->countAll();
            }
            if(this->numPartitions == 0) {
                if(this->parent != nullptr) {
                    this->parent->removeChildPartition(this->partitionId);
                }
            }
            return count;
        }

    }



    bool get(string key, int & value) {
        if (this->numPartitions == 1) {
            try{
                value = this->leafPartition->at(key);
                return true;
            }
            catch (std::out_of_range &e) {
                cout << "Can not find key="<<key<<"\n";
                value = -1;
                return false;
            }
        } else {
            int partitionId = this->getPartitionId(key);
            return this->getPartition(partitionId)->get(key, value);
        }

    }

    void set(string key, int value) {
       if (this->numPartitions == 1) {
           try {
               this->leafPartition->at(key) = value;
               return;
           }
           catch (std::out_of_range &e) {
               this->insert(key, value);
               return;
           }
       } else {
           int partitionId = this->getPartitionId(key);
           this->getPartition(partitionId)->set(key, value);
           return;
       }

    } 

    void closeSpillSet() {
       leafSpillFile->unpinBufferPage();
    }    
 
    //if I am leaf
    void split(PDBPagePtr anotherLeafPage) {
       cout << "split at path:" << this->path << "\n";
       //create a new hash paritition using anotherLeafPage 
       char strPath[500];
       sprintf(strPath, "%s/%d", this->path.c_str(), 1);
       TestVirtualHashMapWithSlabAllocator * partition = new TestVirtualHashMapWithSlabAllocator(1, 1, this->level+1, 1024*1024, strPath, this->storage, this->hashset, this->leafSpillFile, this, anotherLeafPage, nullptr);
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
       sprintf(strPath, "%s/%d", this->path.c_str(), 0);
       TestVirtualHashMapWithSlabAllocator * anotherPartition = new TestVirtualHashMapWithSlabAllocator(1, 0, this->level+1, 0, strPath, this->storage, this->hashset, this->leafSpillFile, this, this->leafPage,  this->leafPartition);
       //upgrade current node from leaf node to an intermediate node;
       this->addPartition(anotherPartition);
       this->addPartition(partition);
       this->leafPage = nullptr;
       this->leafPartition = nullptr;
    }

    void spill() {
        if(this->numPartitions != 1) {
            return;
        }
        else {
            cout << "spill at path:"<< this->path<<"\n";
            /*
            //create a FileSet if needed
            if(this->leafSpillFile == nullptr) {
                this->storage->addFileSet(this->path, this->fileSetId);
                this->leafSpillFile = this->storage->getFileSet(this->fileSetId);
            }
            */
            //iterate the unordered_map, and write the key value pairs to the FileSet
            for (auto it = this->leafPartition->begin(); it != this->leafPartition->end(); ++it) {
                size_t length = ((string)(it->first)).length();
                size_t size = sizeof(int) + length + sizeof(int);
                void * start = this->leafSpillFile->getNewBytes(size, false);
                if(start == nullptr) {
                    cout << "Error: Out of Memory when spill partial aggregation results!\n";
                    exit(-1);
                }
                char * cur = (char *) start;
                * ((int *)cur) = (int)(length);
                cur += sizeof(int);
                memcpy(cur, ((string)(it->first)).c_str(), length);
                cur += length;
                * ((int *)cur) = it->second;
                cur += sizeof(int);
            }
            //refresh the current map
            this->leafPartition->clear();
            delete this->leafPartition;
            void * data = this->leafPage->getRawBytes();
            STLSlabAllocator<pair<string, int>> allocator(data, storage->getConf()->getPageSize());
            hash<string> hasher = hash<string>();
            equal_to<string> key_equal = equal_to<string>();
            this->leafPartition = new unordered_map<string, int, hash<string>, equal_to<string>, STLSlabAllocator<std::pair<string, int>>> (size, hasher, key_equal, allocator);
            if(this->spilled == false) {
                this->spilled = true;
            }
        }
    }

    bool removeChildPartition(int partitionId) {
        if(partitionId >= numPartitions) {
            return false;
        }
        TestVirtualHashMapWithSlabAllocator * child = this->childPartitions->at(partitionId);
        delete child;
        this->childPartitions->erase(this->childPartitions->begin() + partitionId);
        this->numPartitions--;
        return true;
    }
protected:
    /*
     * To get partition id from the key.
     */
    int getPartitionId(string const & key) {
       size_t keyhash = hash<string>()(key) >> this->level;
       return keyhash % this->numPartitions;        
    }

    /*
     * To get partition based on partition id.
     */
    TestVirtualHashMapWithSlabAllocator * getPartition(int index) {
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
    void addPartition(TestVirtualHashMapWithSlabAllocator * partition) {
        if(this->childPartitions == nullptr) {
            this->childPartitions = new vector<TestVirtualHashMapWithSlabAllocator *>();
        }
        this->childPartitions->push_back(partition);
    }
 
    
private:
    int numPartitions;
    int level;
    TestVirtualHashMapWithSlabAllocator * parent;
    vector<TestVirtualHashMapWithSlabAllocator *> * childPartitions;
    string path;
    PageCachePtr cache; 
    PDBStoragePtr storage;
    HashSetPtr hashset;
    int size;
    //if I am leaf   
    //FileSetPtr leafSpillFile;
    TempSetPtr leafSpillFile;
    SetID fileSetId;
    int partitionId;
    PDBPagePtr leafPage; //needs to unpin the page on clear
    unordered_map<string, int, hash<string>, equal_to<string>, STLSlabAllocator<std::pair<string, int>>> * leafPartition;
    bool spilled = false;
};

#endif
