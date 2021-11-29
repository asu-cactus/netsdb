
#ifndef SRC_BUILTINPDBOBJECTS_HEADERS_UNPINPAGE_H_
#define SRC_BUILTINPDBOBJECTS_HEADERS_UNPINPAGE_H_

#include "Object.h"
#include "DataTypes.h"

//  PRELOAD %StorageUnpinPage%

namespace pdb {
// this object type is sent to the server to tell it to unpin a page.
class StorageUnpinPage : public pdb::Object {


public:
    StorageUnpinPage() {}
    ~StorageUnpinPage() {}
    // get/set nodeId
    NodeID getNodeID() {
        return this->nodeId;
    }
    void setNodeID(NodeID nodeId) {
        this->nodeId = nodeId;
    }
    // get/set databaseId
    DatabaseID getDatabaseID() {
        return this->dbId;
    }
    void setDatabaseID(DatabaseID dbId) {
        this->dbId = dbId;
    }
    // get/set userTypeId
    UserTypeID getUserTypeID() {
        return this->userTypeId;
    }
    void setUserTypeID(UserTypeID typeId) {
        this->userTypeId = typeId;
    }
    // get/set setId
    SetID getSetID() {
        return this->setId;
    }
    void setSetID(SetID setId) {
        this->setId = setId;
    }
    // get/set pageId
    PageID getPageID() {
        return this->pageId;
    }
    void setPageID(PageID pageId) {
        this->pageId = pageId;
    }

    // get/set wasDirty
    bool getWasDirty() {
        return this->wasDirty;
    }
    void setWasDirty(bool wasDirty) {
        this->wasDirty = wasDirty;
    }

    ENABLE_DEEP_COPY

private:
    NodeID nodeId = -1;
    DatabaseID dbId = -1;
    UserTypeID userTypeId = -1;
    SetID setId = -1;
    PageID pageId = -1;
    bool wasDirty = true;
};
}


#endif /* SRC_BUILTINPDBOBJECTS_HEADERS_UNPINPAGE_H_ */
