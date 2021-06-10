#ifndef ABSTRACT_HASH_SET
#define ABSTRACT_HASH_SET


#include <memory>

namespace pdb {

class AbstractHashSet;
typedef std::shared_ptr<AbstractHashSet> AbstractHashSetPtr;


/*
 * this class encapsulates an abstract hash set
 * in the end, this class will be replaced by a Panega hash set
 */

class AbstractHashSet {


public:
    // return the hash set type
    virtual std::string getHashSetType() = 0;

    // clean up
    virtual void cleanup() = 0;

    // get size
    virtual size_t getSize() = 0;

    
    // set materialized
    void setMaterialized(bool materialized) {
        this->materialized = materialized;
    }

    //return materialized
    bool isMaterialized() {
        return materialized;
    }


private:

    //is this HashSet materialized?
    bool materialized = false;





};
}


#endif
