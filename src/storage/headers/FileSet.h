

#ifndef FILESET_H
#define FILESET_H

#include "PageCache.h"
#include "PDBLogger.h"
#include "PageIterator.h"
#include "PageCache.h"
#include "DataTypes.h"
#include <memory>
#include <string>
using namespace std;

class FileSet;
typedef shared_ptr<FileSet> FileSetPtr;

class FileSet : public LocalitySet {
public:
    FileSet(PageCachePtr cache,
            string filePath,
            size_t pageSize,
            NodeID nodeId,
            DatabaseID dbId,
            UserTypeID typeId,
            SetID setId);
    ~FileSet();
    void clear();
    int writeData(void* data, size_t length);
    PageIteratorPtr getIterator();
    NodeID getNodeID() {
        return this->nodeId;
    }

    DatabaseID getDatabaseID() {
        return this->dbId;
    }
    UserTypeID getUserTypeID() {
        return this->typeId;
    }
    SetID getSetID() {
        return this->setId;
    }
    string getFilePath() {
        return this->filePath;
    }

private:
    string filePath;
    int handle;
    PageCachePtr cache;
    size_t pageSize;
    size_t fileSize;
    NodeID nodeId;
    DatabaseID dbId;
    UserTypeID typeId;
    SetID setId;
};

#endif
