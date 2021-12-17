
#ifndef STORAGE_ADD_SHARED_PAGE_H
#define STORAGE_ADD_SHARED_PAGE_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"

// PRELOAD %StorageAddSharedPage%

namespace pdb {

// encapsulates a request to add the information regarding a shared page to a set in storage
// i.e., linking a shared page to a set
class StorageAddSharedPage : public Object {

public:
    StorageAddSharedPage() {}
    ~StorageAddSharedPage() {}

    StorageAddSharedPage(std::string sharingDatabase,
                   std::string sharingSetName,
                   std::string sharingTypeName,
		   std::string sharedDatabase,
		   std::string sharedSetName,
		   std::string sharedTypeName,
		   PageID pageId,
		   FilePartitionID filePartitionId,
		   unsigned int pageSeqId,
		   bool whetherToAddSharedSet)
        : sharingDatabase(sharingDatabase), sharingSetName(sharingSetName), sharingTypeName(sharingTypeName),
	  sharedDatabase(sharedDatabase), sharedSetName(sharedSetName), sharedTypeName(sharedTypeName),
	  pageId(pageId), filePartitionId(filePartitionId), pageSeqId(pageSeqId), whetherToAddSharedSet(whetherToAddSharedSet) {
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

    PageID getPageId() {
        return pageId;
    }

    FilePartitionID getFilePartitionId() {
        return filePartitionId;
    }

    unsigned int getPageSeqId() {
        return pageSeqId;
    }

    bool getWhetherToAddSharedSet() {
        return whetherToAddSharedSet;
    }


    ENABLE_DEEP_COPY

private:
    String sharingDatabase;
    String sharingSetName;
    String sharingTypeName;
    String sharedDatabase;
    String sharedSetName;
    String sharedTypeName;
    PageID pageId;
    FilePartitionID filePartitionId;
    unsigned int pageSeqId;
    bool whetherToAddSharedSet;
};
}

#endif
