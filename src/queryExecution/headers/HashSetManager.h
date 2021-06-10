#ifndef HASH_SET_MANAGER
#define HASH_SET_MANAGER


#include "AbstractHashSet.h"

namespace pdb {

/*
 * encapsulates a hash set manager to manage hash tables allocated on heap
 * this functionality will be replaced by Pangea storage manger
 */

class HashSetManager {

private:
    // all hash tables allocated
    std::map<std::string, AbstractHashSetPtr> hashSets;
    size_t totalSize = 0;

public:
    // to get a hash set
    AbstractHashSetPtr getHashSet(std::string name) {
        if (hashSets.count(name) == 0) {
            return nullptr;
        } else {
            return hashSets[name];
        }
    }

    // to add a hash set
    bool addHashSet(std::string name, AbstractHashSetPtr hashSet) {
        if (hashSets.count(name) != 0) {
            std::cout << "Error: hash set exists: " << name << std::endl;
            return false;
        } else {
            hashSets[name] = hashSet;
            if (hashSet != nullptr) {
                totalSize += hashSet->getSize();
            }
            return true;
        }
    }

    //to know whether a set is materialized
    bool isMaterialized(std::string name) {
        if (hashSets.count(name) == 0) {
            std::cout << "Error: hash set doesn't exist: " << name << std::endl;
            return false;
        } else {
            return hashSets[name]->isMaterialized();
        }

    }

    // to remove a hash set
    bool removeHashSet(std::string name) {
        if (hashSets.count(name) == 0) {
            std::cout << "Error: hash set doesn't exist: " << name << std::endl;
            return false;
        } else {
            totalSize -= hashSets[name]->getSize();
            hashSets.erase(name);
            return true;
        }
    }



    // get total size
    size_t getTotalSize() {
        auto a = hashSets.begin();
        size_t curSetSize = 0;
        while (a != hashSets.end()) {
            curSetSize += a->second->getSize();
            std::cout << "current hash set name: " << a->first << " and size: " << a->second->getSize() << std::endl;
            ++a;
        }
        return curSetSize;
    }
};
}


#endif
