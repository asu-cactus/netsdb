
/*
 * SharedTensorBlockSet is a special type of set that stores the shared tensor blocks.
 */


#ifndef SHARED_TENSOR_BLOCK_SET_H
#define SHARED_TENSOR_BLOCK_SET_H

#include "PageCache.h"
#include "PDBLogger.h"
#include "DataTypes.h"
#include "UserSet.h"
#include <memory>
#include <string>
using namespace std;

class SharedTensorBlockSet;
typedef shared_ptr<SharedTensorBlockSet> SharedTensorBlockSetPtr;

class SharedTensorBlockSet : public UserSet {
public:
    SharedTensorBlockSet(pdb::PDBLoggerPtr logger,
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
            size_t desiredSize = 1){
    
       super(logger, shim, nodeId, dbId, typeId, setId, setName,
		       pageCache, localityType, policy, operation, durability,
		       persistence, pageSize, desiredSize);
        
    }

    SharedTensorBlockSet(size_t pageSize,
            pdb::PDBLoggerPtr logger,
            SharedMemPtr shm,
            NodeID nodeId,
            DatabaseID dbId,
            UserTypeID typeId,
            SetID setId,
            string setName,
            PartitionedFilePtr file,
	    std::string pathToIndexFile,
            PageCachePtr pageCache,
            LocalityType localityType = JobData,
            LocalitySetReplacementPolicy policy = LRU,
            OperationType operation = Read,
            DurabilityType durability = TryCache,
            PersistenceType persistence = Persistent,
            size_t desiredSize = 1) {
    
       super(pageSize, logger, shm, nodeId, dbId, typeId, setId, setName,
		       file, pageCache, localityType, policy, operation, durability, 
		       persistence, desiredSize);
       indexes.deserializeIndex(pathToIndexFile);
    }



    void serializeIndex (std::string path) {
        indexes.serializeIndex(path);
    }

    void deserializeIndex(std::string path) {
        indexes.deserializeIndex(path);
    }

    Handle<TensorBlockMeta> getTargetMetadata (DatabaseID dbId, UserTypeID typeId, SetID setId, TensorBlockMeta sourceBlockMeta) {
        return indexes.getTargetMetadata(dbId, typeId, setId, sourceBlockMeta);
    }

    bool insertIndex(DatabaseID dbId, UserTypeID typeId, SetID setId, TensorBlockMeta sourceBlockMeta, Handle<TensorBlockMeta> targetBlockMeta) {
	SetKey key = indexes.getSetKey(dbId, typeId, setId);
	return indexes.insertIndex(key, sourceBlockMeta, targetBlockMeta);
    }

    bool removeIndex(DatabaseID dbId, UserTypeID typeId, SetID setId, TensorBlockMeta sourceBlockMeta) {
        SetKey key = indexes.getSetKey(dbId, typeId, setId);
	return indexes.removeIndex(key, sourceBlockMeta);
    }

private:
    TensorBlockIndex indexes;

};

#endif
