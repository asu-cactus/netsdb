
#ifndef LOCALITY_SET_CC
#define LOCALITY_SET_CC

#include "LocalitySet.h"
#include <iostream>
LocalitySet::LocalitySet(LocalityType localityType,
                         LocalitySetReplacementPolicy replacementPolicy,
                         OperationType operationType,
                         DurabilityType durabilityType,
                         PersistenceType persistenceType,
                         size_t desiredSize,
			 bool isShared) {

    cachedPages = new list<PDBPagePtr>();
    this->localityType = localityType;
    this->replacementPolicy = replacementPolicy;
    this->operationType = operationType;
    this->durabilityType = durabilityType;
    this->persistenceType = persistenceType;
    this->lifetimeEnded = false;
    this->desiredSize = desiredSize;
    this->isShared = isShared;
    pthread_mutex_init(&localitySetCacheMutex, nullptr);

}

LocalitySet::~LocalitySet() {
    cachedPages->clear();
    delete cachedPages;
}

void LocalitySet::addCachedPage(PDBPagePtr page) {
    cachedPages->push_back(page);
}

void LocalitySet::updateCachedPage(PDBPagePtr page) {
    for (list<PDBPagePtr>::iterator it = cachedPages->begin(); it != cachedPages->end(); ++it) {
        if ((*it) == page) {
            pthread_mutex_lock(&localitySetCacheMutex);
            cachedPages->erase(it);
            pthread_mutex_unlock(&localitySetCacheMutex);
            break;
        }
    }
    pthread_mutex_lock(&localitySetCacheMutex);
    cachedPages->push_back(page);
    pthread_mutex_unlock(&localitySetCacheMutex);
}

void LocalitySet::removeCachedPage(PDBPagePtr page) {
    for (list<PDBPagePtr>::iterator it = cachedPages->begin(); it != cachedPages->end(); ++it) {
        if ((*it) == page) {
            cachedPages->erase(it);
            break;
        }
    }
}

bool LocalitySet::getShared() {
   return isShared;
}



PDBPagePtr LocalitySet::selectPageForReplacement() {
    PDBPagePtr retPage = nullptr;
    if (this->replacementPolicy == MRU) {
        for (list<PDBPagePtr>::reverse_iterator it = cachedPages->rbegin();
             it != cachedPages->rend();
             ++it) {
            if ((*it)->getRefCount() == 0) {
                retPage = (*it);
                break;
            }
        }
    } else {
        for (list<PDBPagePtr>::iterator it = cachedPages->begin(); it != cachedPages->end(); ++it) {
            if ((*it)->getRefCount() == 0) {
                retPage = (*it);
                break;
            }
        }
    }
    return retPage;
}

vector<PDBPagePtr>* LocalitySet::selectPagesForReplacement() {
    vector<PDBPagePtr>* retPages = new vector<PDBPagePtr>();
    int totalPages = cachedPages->size();
    if (totalPages == 0) {
        delete retPages;
        return nullptr;
    }
    int numPages = 0;
    if (this->replacementPolicy == MRU) {
        for (list<PDBPagePtr>::reverse_iterator it = cachedPages->rbegin();
             it != cachedPages->rend();
             ++it) {
            if ((*it)->getRefCount() == 0) {
                retPages->push_back(*it);
                numPages++;
                if (this->operationType == Write) {
                    break;
                } else {
                    if (numPages == 1) {
                    /*if ((double)numPages / (double)totalPages >= (1-EVICT_STOP_THRESHOLD)) {*/
                        break;
                    }
                }
            }
        }
    } else {
        for (list<PDBPagePtr>::iterator it = cachedPages->begin(); it != cachedPages->end(); ++it) {
            if ((*it)->getRefCount() == 0) {
                retPages->push_back(*it);
                numPages++;
                if (this->operationType == Write) {
                    break;
                } else {
                    /*if (numPages == 1) {*/
                    if ((double)numPages / (double)totalPages >= (1-EVICT_STOP_THRESHOLD)) {
                        break;
                    }
                }
            }
        }
    }
    if (numPages == 0) {
        delete retPages;
        return nullptr;
    } else {
        return retPages;
    }
}


void LocalitySet::pin(LocalitySetReplacementPolicy policy, OperationType operationType) {
    this->replacementPolicy = policy;
    this->operationType = operationType;
    this->lifetimeEnded = false;
}

void LocalitySet::unpin() {
    this->lifetimeEnded = true;
}


LocalityType LocalitySet::getLocalityType() {
    return this->localityType;
}

void LocalitySet::setLocalityType(LocalityType type) {
    this->localityType = type;
}

LocalitySetReplacementPolicy LocalitySet::getReplacementPolicy() {
    return this->replacementPolicy;
}

void LocalitySet::setReplacementPolicy(LocalitySetReplacementPolicy policy) {
    this->replacementPolicy = policy;
}

OperationType LocalitySet::getOperationType() {
    return this->operationType;
}

void LocalitySet::setOperationType(OperationType type) {
    this->operationType = type;
}


DurabilityType LocalitySet::getDurabilityType() {
    return this->durabilityType;
}

void LocalitySet::setDurabilityType(DurabilityType type) {
    this->durabilityType = type;
}

PersistenceType LocalitySet::getPersistenceType() {
    return this->persistenceType;
}

void LocalitySet::setPersistenceType(PersistenceType type) {
    this->persistenceType = type;
}

bool LocalitySet::isLifetimeEnded() {
    return this->lifetimeEnded;
}

void LocalitySet::setLifetimeEnd(bool lifetimeEnded) {
    this->lifetimeEnded = lifetimeEnded;
}


size_t LocalitySet::getDesiredSize() {
    return this->desiredSize;
}

void LocalitySet::setDesiredSize(size_t desiredSize) {
    this->desiredSize = desiredSize;
}


#endif
