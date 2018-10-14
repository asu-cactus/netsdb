
#ifndef FILESETPAGEITERATOR_H
#define FILESETPAGEITERATOR_H

#include "FileSet.h"
#include "DataTypes.h"
#include "PageCache.h"
#include "PageIterator.h"
#include <memory>
using namespace std;
class FileSetPageIterator;

typedef shared_ptr<FileSetPageIterator> FileSetPageIteratorPtr;

class FileSetPageIterator : public PageIteratorInterface {
public:
    FileSetPageIterator(PageCachePtr cache,
                        int fileHandle,
                        size_t fileSize,
                        size_t pageSize,
                        NodeID nodeId,
                        DatabaseID dbId,
                        UserTypeID typeId,
                        SetID setId);
    virtual ~FileSetPageIterator();
    PDBPagePtr next() override;
    bool hasNext() override;
    bool offsetIteratedSize(size_t iteratedSize);

private:
    PageCachePtr cache;
    int handle;
    size_t fileSize;
    size_t pageSize;
    size_t iteratedSize;
    NodeID nodeId;
    DatabaseID dbId;
    UserTypeID typeId;
    SetID setId;
    PageID pageId;
};

#endif
