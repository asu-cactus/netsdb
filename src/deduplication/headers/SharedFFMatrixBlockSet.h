
/*
 * SharedFFMatrixBlockSet is a special type of set that stores the shared tensor blocks.
 */


#ifndef SHARED_FF_MATRIX_BLOCK_SET_H
#define SHARED_FF_MATRIX_BLOCK_SET_H

#include "PageCache.h"
#include "PDBLogger.h"
#include "DataTypes.h"
#include "UserSet.h"
#include "FFMatrixMeta.h"
#include "FFMatrixBlockIndex.h"
#include <algorithm>    // copy
#include <iterator>
#include <fstream>      // fstream
#include <boost/tokenizer.hpp>
#include <memory>
#include <string>
#include <stdlib.h>

using namespace std;
using namespace boost;

namespace pdb{

class SharedFFMatrixBlockSet;
typedef std::shared_ptr<SharedFFMatrixBlockSet> SharedFFMatrixBlockSetPtr;

class SharedFFMatrixBlockSet : public UserSet {
public:


    SharedFFMatrixBlockSet(size_t pageSize, pdb::PDBLoggerPtr logger,
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

    SharedFFMatrixBlockSet(size_t pageSize,
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

    Handle<FFMatrixMeta> getTargetMetadata (DatabaseID dbId, UserTypeID typeId, SetID setId, int blockRowId, int blockColId) {
        return indexes.getTargetMetadata(dbId, typeId, setId, blockRowId, blockColId);
    }

    //each line of index file is like following:
    //sharedBlockRowIndex, sharedBlockColIndex, actualBlockRowIndex, actualBlockColIndex
    void loadIndexFromFile(DatabaseID dbId, UserTypeID typeId, SetID setId, std::string path, size_t totalRows, size_t totalCols) {

         ifstream in(path.c_str());
         if (!in.is_open()) {
	   std::cout << "FATAL ERROR: Cannot open file: " << path << std::endl;	 
           return;
         }
	 typedef boost::tokenizer< escaped_list_separator<char> > Tokenizer;
	 std::string line;
	 std::vector< std::string > vec;
         while (getline(in,line)) {
             Tokenizer tok(line);
             vec.assign(tok.begin(),tok.end());
             Handle<FFMatrixMeta> actualBlockMeta = makeObject<FFMatrixMeta>(atoi(vec[2].c_str()), atoi(vec[3].c_str()), totalRows, totalCols);
	     insertIndex(dbId, typeId, setId, atoi(vec[0].c_str()), atoi(vec[1].c_str()), actualBlockMeta);
         }
    }
    

    bool insertIndex(DatabaseID dbId, UserTypeID typeId, SetID setId, int blockColId, int blockRowId, Handle<FFMatrixMeta> targetBlockMeta) {
	size_t key = indexes.getSetKey(dbId, typeId, setId);
	return indexes.insertIndex(key, blockColId, blockRowId, targetBlockMeta);
    }

    bool removeIndex(DatabaseID dbId, UserTypeID typeId, SetID setId, int blockColId, int blockRowId) {
        size_t key = indexes.getSetKey(dbId, typeId, setId);
	return indexes.removeIndex(key, blockColId, blockRowId);
    }

private:
    FFMatrixBlockIndex indexes;

};

}

#endif
