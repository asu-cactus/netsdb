//
// Created by Joseph Hwang on 9/12/16.
//

#ifndef OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEADDINDEXER_H
#define OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEADDINDEXER_H

#include "Object.h"
#include "Handle.h"
#include "PDBString.h"
#include "AbstractIndexer.h"

// PRELOAD %DistributedStorageAddIndexer%

namespace pdb {

// encapsulates a request to add data to a set in storage
class DistributedStorageAddIndexer : public Object {

public:
    DistributedStorageAddIndexer() {}
    ~DistributedStorageAddIndexer() {}

    DistributedStorageAddIndexer(std::string databaseName,
                      std::string typeName,
                      int16_t typeId,
                      Handle<AbstractIndexer> indexer)
        : databaseName(databaseName), typeName(typeName), typeId(typeId), indexer(indexer) {
    }

    std::string getDatabaseName() {
        return databaseName;
    }

    std::string getTypeName() {
        return typeName;
    }

    int getTypeId() {
        return typeId;
    }

    Handle<AbstractIndexer> getIndexer() {
        return indexer;
    }

    ENABLE_DEEP_COPY

private:
    String databaseName;
    String typeName;
    int typeId;
    Handle<AbstractIndexer> indexer;
};
}

#endif  // OBJECTQUERYMODEL_DISTRIBUTEDSTORAGEADDINDEXER_H
