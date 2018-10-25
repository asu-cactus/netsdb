

#ifndef SET_CACHE_PAGE_ITERATOR_CC
#define SET_CACHE_PAGE_ITERATOR_CC

#include "PDBDebug.h"
#include "SetCachePageIterator.h"
// TODO using snapshot and reference count to support multiple concurrent iterators for one same
// buffer
// Argument: why do we need multiple concurrent iterators for the same buffer?
// Maybe there are two concurrent jobs? so can we just share the iterator between two jobs?

// thread-safe (Fixed by Jia: now flushing is OK by updating the FileSearchKey)

// NOTE: the constructor can only be invoked in UserSet::getIterators(), where it will be protected
// by lockDirtyPageSet();

SetCachePageIterator::SetCachePageIterator(PageCachePtr cache, UserSet* set) {
    this->cache = cache;
    this->set = set;
    this->iter = this->set->getDirtyPageSet()->begin();
}

// remove all elements that have been flushed to disk (inCache == false)
SetCachePageIterator::~SetCachePageIterator() {}

PDBPagePtr SetCachePageIterator::begin() {
    this->iter = this->set->getDirtyPageSet()->begin();
    return nullptr;
}

PDBPagePtr SetCachePageIterator::end() {
    this->iter = this->set->getDirtyPageSet()->end();
    return nullptr;
}

PDBPagePtr SetCachePageIterator::next() {
    if (this->iter != this->set->getDirtyPageSet()->end()) {
        if (this->iter->second.inCache == true) {
            CacheKey key;
            key.dbId = this->set->getDbID();
            key.typeId = this->set->getTypeID();
            key.setId = this->set->getSetID();
            key.pageId = this->iter->first;
            std::cout << "SetCachePageIterator: in cache: curPageId=" << key.pageId << "\n";
#ifdef USE_LOCALITY_SET
            PDBPagePtr page = this->cache->getPage(key, this->set);
#else
            PDBPagePtr page = this->cache->getPage(key, nullptr);
#endif
            ++iter;
            return page;
        } else {
            // the page is already flushed to file, so load from file
            PageID pageId = this->iter->first;
            std::cout << "SetCachePageIterator: not in cache: curPageId=" << pageId << "\n";
            FileSearchKey searchKey = this->iter->second;
#ifdef USE_LOCALITY_SET
            PDBPagePtr page = this->cache->getPage(this->set->getFile(),
                                                   searchKey.partitionId,
                                                   searchKey.pageSeqInPartition,
                                                   pageId,
                                                   false,
                                                   this->set);
#else
            PDBPagePtr page = this->cache->getPage(this->set->getFile(),
                                                   searchKey.partitionId,
                                                   searchKey.pageSeqInPartition,
                                                   pageId,
                                                   false,
                                                   nullptr);
#endif
            // remove iter
            this->set->lockDirtyPageSet();
            this->iter = this->set->getDirtyPageSet()->erase(this->iter);
            this->set->unlockDirtyPageSet();
            return page;
        }
    }
    return nullptr;
}

bool SetCachePageIterator::hasNext() {
    if (this->iter != this->set->getDirtyPageSet()->end()) {
        return true;
    } else {
        return false;
    }
}

#endif
