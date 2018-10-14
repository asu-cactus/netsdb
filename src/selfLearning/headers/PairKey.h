#ifndef PAIR_KEY_H
#define PAIR_KEY_H

namespace pdb {

/**
 * Hash function for PairKey.
 */

struct PairKeyHash {

    std::size_t operator()(const std::pair<long, long> & key) const {
        return (key.first << 16) + key.second;
    }
};

/**
 * Comparator for CacheKey, used for caching and retrieving a page.
 */

struct PairKeyEqual {

    bool operator()(const std::pair<long, long>& lKey, const std::pair<long, long>& rKey) const {
        if ((lKey.first == rKey.first) && (lKey.second == rKey.second))  {
            return true;
        } else {
            return false;
        }
    }
};


}

#endif
