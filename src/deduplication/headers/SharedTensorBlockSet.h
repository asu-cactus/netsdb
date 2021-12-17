
/*
 * SharedTensorBlockSet is a special type of set that stores the shared tensor blocks.
 */


#ifndef SHARED_TENSOR_BLOCK_SET_H
#define SHARED_TENSOR_BLOCK_SET_H

#include "PageCache.h"
#include "PDBLogger.h"
#include "DataTypes.h"
#include "UserSet.h"
#include "TensorBlockMeta.h"
#include "TensorBlockIndex.h"
#include <memory>
#include <string>
using namespace std;

namespace pdb{

class SharedTensorBlockSet;
typedef shared_ptr<SharedTensorBlockSet> SharedTensorBlockSetPtr;

class SharedTensorBlockSet : public UserSet {
public:


    SharedTensorBlockSet(size_t pageSize, pdb::PDBLoggerPtr logger,
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
            size_t desiredSize = 1): UserSet(pageSize, logger, shm, nodeId, dbId, typeId, setId, setName,
		       file, pageCache, localityType, policy, operation, durability,
		       persistence, desiredSize, true) {
		    std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
		    std::cout << "Created shared set:" << setName << std::endl;
		    if (isShared) {
		       std::cout << "isShared set to true" << std::endl;
		    }
		    std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
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
            size_t desiredSize = 1) : UserSet(pageSize, logger, shm, nodeId, dbId, typeId, setId, setName,
		       file, pageCache, localityType, policy, operation, durability, 
		       persistence, desiredSize, true) {
       indexes.deserializeIndex(pathToIndexFile);
                           std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
                    std::cout << "Created shared set:" << setName << std::endl;
                    if (isShared) {
                       std::cout << "isShared set to true" << std::endl;
                    }
                    std::cout << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;

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
	size_t key = indexes.getSetKey(dbId, typeId, setId);
	return indexes.insertIndex(key, sourceBlockMeta, targetBlockMeta);
    }

    bool removeIndex(DatabaseID dbId, UserTypeID typeId, SetID setId, TensorBlockMeta sourceBlockMeta) {
        size_t key = indexes.getSetKey(dbId, typeId, setId);
	return indexes.removeIndex(key, sourceBlockMeta);
    }

private:
    TensorBlockIndex indexes;

};

}

#endif
