
/*
 * FFMatrixBlockIndex is a serielizable index that maps the metadata of the persist deduplicated tensor blocks (i.e., sources)
 * to the metadata of runtime tensor blocks (i.e., targets).
 * The key will be a FFMatrixBlockIdentifier that specifies the target set and the source block.
 * The value will be a Vector<int> that specifies the target block metadata.
 */


#ifndef FF_MATRIX_BLOCK_INDEX_H
#define FF_MATRIX_BLOCK_INDEX_H

#include "FFMatrixMeta.h"
#include "FFMatrixBlock.h"
#include "PDBMap.h"

#include <cstddef>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <cstring>
#include <ctime>
#include <chrono>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <cmath>

using namespace std;

namespace pdb {

class FFMatrixBlockIndex  {
public:
    FFMatrixBlockIndex() {
       indexes = makeObject<Map<size_t, Handle<Map<FFMatrixMeta, Handle<FFMatrixMeta>>>>>();
    }

    FFMatrixBlockIndex(std::string path) {
       deserializeIndex(path);
    }


    ~FFMatrixBlockIndex() {
       indexes = nullptr;
    }

    void serializeIndex (std::string path) {
        int filedesc = open(path.c_str(), O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
        myBytes =
            getRecord<Map<size_t, Handle<Map<FFMatrixMeta, Handle<FFMatrixMeta>>>>>(indexes);
        write(filedesc, myBytes, myBytes->numBytes());
        close(filedesc);
        std::cout << "Wrote " << myBytes->numBytes() << " bytes to the file.\n";
    }

    void deserializeIndex (std::string path) {
        std::ifstream in(path.c_str(), std::ifstream::ate | std::ifstream::binary);
        size_t fileLen = in.tellg();
        int filedesc = open(path.c_str(), O_RDONLY);
        myBytes = (Record<Map<size_t, Handle<Map<FFMatrixMeta, Handle<FFMatrixMeta>>>>>*)malloc(fileLen);
        read(filedesc, myBytes, fileLen);
        close(filedesc);
        indexes = myBytes->getRootObject();
    }

    Handle<FFMatrixMeta> getTargetMetadata (DatabaseID dbId, UserTypeID typeId, SetID setId, FFMatrixMeta sourceBlockMeta) {
        size_t setKey = getSetKey(dbId, typeId, setId);
	if (indexes->count(setKey) > 0) {
	    Handle<Map<FFMatrixMeta, Handle<FFMatrixMeta>>>  metaMap = (*indexes)[setKey];
	    if(metaMap->count(sourceBlockMeta) > 0) {
	        return (*metaMap)[sourceBlockMeta];
	    } 
	}
	return nullptr;
        
    }

    bool insertIndex(int setKey, FFMatrixMeta sourceBlockMeta, Handle<FFMatrixMeta> targetBlockMeta) {
        if (indexes->count(setKey) == 0) {

	    (*indexes)[setKey] = makeObject<Map<FFMatrixMeta, Handle<FFMatrixMeta>>>();
	}
    
	if (indexes->count(setKey) > 0) {
	    Handle<Map<FFMatrixMeta, Handle<FFMatrixMeta>>>  metaMap = (*indexes)[setKey];
            (*metaMap)[sourceBlockMeta] = targetBlockMeta;
	    return true;   
	}
	return false;
    }

    bool removeIndex(int setKey, FFMatrixMeta sourceBlockMeta) {
       if (indexes->count(setKey) > 0) {
            Handle<Map<FFMatrixMeta, Handle<FFMatrixMeta>>>  metaMap = (*indexes)[setKey];
            metaMap->setUnused(sourceBlockMeta);
            return true;
        }
        return false;
    }

    size_t getSetKey(DatabaseID dbId, UserTypeID typeId, SetID setId) {
        size_t code = (size_t)typeId + (size_t)setId * (size_t)100000 + (size_t)dbId * (size_t)100000000;
	std::cout << "code = " << code << std::endl;
	return code;
    }

private:
    Handle<Map<size_t, Handle<Map<FFMatrixMeta, Handle<FFMatrixMeta>>>>> indexes = nullptr;
    Record<Map<size_t, Handle<Map<FFMatrixMeta, Handle<FFMatrixMeta>>>>>* myBytes = nullptr;
};

}

#endif
