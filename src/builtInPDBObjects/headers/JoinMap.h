
#ifndef JOIN_MAP_H
#define JOIN_MAP_H

// PRELOAD %JoinMap <Nothing>%

#include "Object.h"
#include "Handle.h"
#include "JoinPairArray.h"

template <typename StoredType>
class JoinRecordLst;

namespace pdb {

// This is the Map type used to power joins

template <class ValueType>
class JoinMap : public Object {

private:
    // this is where the data are actually stored
    Handle<JoinPairArray<ValueType>> myArray;

    // size of ValueType
    size_t objectSize;

    // my partition id
    size_t partitionId;

    // number of partitions (per node)
    int numPartitions;

public:
    ENABLE_DEEP_COPY

    // this constructor creates a map with specified slots, partitionId and numPartitions
    JoinMap(uint32_t initSize, size_t partitionId, int numPartitions);

    // this constructor pre-allocates initSize slots... initSize must be a power of two
    JoinMap(uint32_t initSize);

    // this constructor creates a map with a single slot
    JoinMap();

    // destructor
    ~JoinMap();

    // allows us to access all of the records with a particular hash value
    JoinRecordList<ValueType> lookup(const size_t& which);

    // adds a new value at position which
    ValueType& push(const size_t& which);

    // clears the particular key from the map, destructing both the key and the value.
    // This is typically used when an out-of-memory
    // exception is thrown when we try to add to the hash table, and we want to immediately clear
    // the last item added.
    void setUnused(const size_t& clearMe);

    // returns the number of elements in the map
    size_t size() const;

    // returns 0 if this entry is undefined; 1 if it is defined
    int count(const size_t& which);

    // these are used for iteration
    JoinMapIterator<ValueType> begin();
    JoinMapIterator<ValueType> end();


    // JiaNote: add partition id to enable hash partitioned join
    size_t getPartitionId();
    void setPartitionId(size_t partitionId);
    int getNumPartitions();
    void setNumPartitions(int numPartitions);

    // JiaNote: add this to enable combination of two JoinMaps
    size_t getObjectSize();
    void setObjectSize();
    void print();
    // this method is used to do a deep copy of this join map
    template <class T> friend Handle<JoinMap<T>> deepCopyJoinMap(Handle<JoinMap<T>>& copyMe);


};

template <class JoinMapType>
Handle<JoinMap<JoinMapType>> deepCopyJoinMap(Handle<JoinMap<JoinMapType>>& copyMe) {


    // create an empty join map
    JoinMapRecordClass<JoinMapType> temp;

    // create a new map
    Handle<JoinMap<JoinMapType>> joinMapToReturn = makeObject<JoinMap<JoinMapType>>();

    // copy the fields
    joinMapToReturn->objectSize = copyMe->objectSize;
    joinMapToReturn->partitionId = copyMe->partitionId;
    joinMapToReturn->numPartitions = copyMe->numPartitions;

    // allocate an array with extra space to hold the joinMap
    auto copyThisArray = ((JoinPairArray<JoinMapType>*) copyMe->myArray.getTarget()->getObject());
    auto size = copyThisArray->getSize(copyThisArray) - sizeof(JoinPairArray<Nothing>);
    joinMapToReturn->myArray = makeObjectWithExtraStorage<JoinPairArray<JoinMapType>>(size);

    JoinPairArray<JoinMapType>& fromMe = *(copyThisArray);
    JoinPairArray<JoinMapType>& toMe = *(joinMapToReturn->myArray.getTarget()->getObject());

    // copy the number of slots
    toMe.numSlots = fromMe.numSlots;
    toMe.usedSlots = fromMe.usedSlots;

    // copy the type info
    toMe.valueTypeInfo = fromMe.valueTypeInfo;

    // copy over the size info
    toMe.objSize = fromMe.objSize;
    toMe.maxSlots = fromMe.maxSlots;

    // copy over the overflow area
    new ((void*) &toMe.overflows) Vector<Vector<JoinMapType>>();
    for(int i = 0; i < fromMe.overflows.size(); ++i) {

      // grab one of the overflow buckets
      auto &bucket = fromMe.overflows[i];

      // figure out how large the array is
      auto howMany = bucket.myArray->numSlots;

      // allocate the new Array
      Handle<Array<JoinMapType>> tempArray = makeObjectWithExtraStorage<Array<JoinMapType>>(sizeof(JoinMapType) * howMany, howMany);

      // and this gives us our info about TypeContained
      tempArray->typeInfo = bucket.myArray->typeInfo;

      // the number of slots actually used
      tempArray->usedSlots = bucket.myArray->usedSlots;

      // the number of slots
      tempArray->numSlots = bucket.myArray->numSlots;

      // copy each join tuple
      for(int j = 0; j < tempArray->usedSlots; j++) {
        new ((void*) & (tempArray->c_ptr()[j])) JoinMapType();
        tempArray->c_ptr()[j] = (bucket.c_ptr())[j];
      }

      // store the vector
      toMe.overflows.push_back();
      toMe.overflows[i].myArray = tempArray;
    }

    // this is needed by the macros used below
    uint32_t objSize = toMe.objSize;

    // loop through and do the deep copy
    for (int i = 0; i < toMe.numSlots; i++) {

        // copy over the hash for this guy
        JM_GET_HASH(toMe.data, i) = JM_GET_HASH(fromMe.data, i);

        // don't copy over an unused pos
        if (JM_GET_HASH(fromMe.data, i) == JM_UNUSED)
            continue;

        JM_GET_NEXT(toMe.data, i) = JM_GET_NEXT(fromMe.data, i);

        new (JM_GET_VALUE_PTR(toMe.data, i)) JoinMapType();
        *((JoinMapType*) JM_GET_VALUE_PTR(toMe.data, i)) = *((JoinMapType*) JM_GET_VALUE_PTR(fromMe.data, i));
    }

    return joinMapToReturn;
}



}

#include "JoinMap.cc"

#endif
