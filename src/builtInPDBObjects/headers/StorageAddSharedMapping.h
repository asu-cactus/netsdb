
#ifndef STORAGE_ADD_SHARED_MAPPING_H
#define STORAGE_ADD_SHARED_MAPPING_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageAddSharedMapping%

namespace pdb {

// encapsulates a request to add the information regarding how a shared block get mapped to an actual block in a set in storage
// i.e., linking a shared block to a set block
class StorageAddSharedMapping : public Object {

public:
    StorageAddSharedMapping() {}
    ~StorageAddSharedMapping() {}

    StorageAddSharedMapping(std::string sharingDatabase,
                   std::string sharingSetName,
                   std::string sharingTypeName,
		   std::string sharedDatabase,
		   std::string sharedSetName,
		   std::string sharedTypeName,
		   std::string fileName,
		   size_t totalRows,
		   size_t totalCols,
		   bool transpose)
        : sharingDatabase(sharingDatabase), sharingSetName(sharingSetName), sharingTypeName(sharingTypeName),
	  sharedDatabase(sharedDatabase), sharedSetName(sharedSetName), sharedTypeName(sharedTypeName),
	  fileName(fileName), totalRows(totalRows), totalCols(totalCols), transpose(transpose) {

    }


    std::string getSharingDatabase() {
        return sharingDatabase;
    }

    std::string getSharingSetName() {
        return sharingSetName;
    }

    std::string getSharingType() {
        return sharingTypeName;
    }

    std::string getSharedDatabase() {
        return sharedDatabase;
    }

    std::string getSharedSetName() {
        return sharedSetName;
    }

    std::string getSharedType() {
        return sharedTypeName;
    }

    std::string getFileName() {
        return fileName;
    }

    size_t getTotalRows() {
        return totalRows;
    }

    size_t getTotalCols() {
        return totalCols;
    }

    bool getTranspose() {
        return transpose;
    }

    ENABLE_DEEP_COPY

private:
    String sharingDatabase;
    String sharingSetName;
    String sharingTypeName;
    String sharedDatabase;
    String sharedSetName;
    String sharedTypeName;
    String fileName;
    size_t totalRows;
    size_t totalCols;
    bool transpose;
};
}

#endif
